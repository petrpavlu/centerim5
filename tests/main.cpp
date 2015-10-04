#include <cppconsui/InputProcessor.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/CppConsUI.h>
#include <cppconsui/KeyConfig.h>

#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <signal.h>
#include <sstream>
#include <unistd.h>

// Function that sets up actual test windows and widgets.
void setupTest();

// TestApp class
class TestApp : public CppConsUI::InputProcessor {
public:
  static int run();

private:
  static TestApp *my_instance_;

  bool exit_now_;
  bool resize_pending_;
  int resize_pipe_[2];

  void redraw();
  void logDebug(const char *message);
  void quit();

  int initializeScreenResizing(std::ostringstream &error_stream);
  int finalizeScreenResizing(std::ostringstream &error_stream);

  static void sigwinch_handler_(int signum)
  {
    my_instance_->sigwinch_handler(signum);
  }
  void sigwinch_handler(int signum);

  TestApp();
  virtual ~TestApp() override {}
  int runAll();

  CONSUI_DISABLE_COPY(TestApp);
};

TestApp *TestApp::my_instance_ = NULL;

int TestApp::run()
{
  // Initialize TestApp instance.
  assert(my_instance_ == nullptr);
  my_instance_ = new TestApp;

  // Run the program.
  int res = my_instance_->runAll();

  // Finalize the instance.
  assert(my_instance_ != nullptr);

  delete my_instance_;
  my_instance_ = nullptr;

  return res;
}

void TestApp::redraw()
{
  // Ignore the redraw event. CoreManager is queried if the redraw is pending
  // in each mainloop cycle.
}

void TestApp::logDebug(const char * /*message*/)
{
  // Ignore all messages.
}

void TestApp::quit()
{
  exit_now_= true;
}

int TestApp::initializeScreenResizing(std::ostringstream &error_stream)
{
  // Create a self-pipe.
  assert(resize_pipe_[0] == -1);
  assert(resize_pipe_[1] == -1);

  int res = pipe(resize_pipe_);
  if (res != 0) {
    error_stream << "Creating a self-pipe for screen resizing failed: " <<
      std::strerror(errno) << '\n';
    return 1;
  }

  // Set close-on-exec on both descriptors.
  res = fcntl(resize_pipe_[0], F_GETFD);
  assert(res != -1);
  res = fcntl(resize_pipe_[0], F_SETFD, res | FD_CLOEXEC);
  assert(res == 0);
  res = fcntl(resize_pipe_[1], F_GETFD);
  assert(res != -1);
  res = fcntl(resize_pipe_[1], F_SETFD, res | FD_CLOEXEC);
  assert(res == 0);

  // Register a SIGWINCH handler.
  struct sigaction sig;
  sig.sa_handler = sigwinch_handler_;
  sig.sa_flags = SA_RESTART;
  res = sigemptyset(&sig.sa_mask);
  assert(res == 0);
  res = sigaction(SIGWINCH, &sig, nullptr);
  assert(res == 0);

  return 0;
}

int TestApp::finalizeScreenResizing(std::ostringstream &error_stream)
{
  int res = 0;

  // Unregister the SIGWINCH handler.
  struct sigaction sig;
  sig.sa_handler = SIG_DFL;
  sig.sa_flags = 0;
  res = sigemptyset(&sig.sa_mask);
  assert(res == 0);
  res = sigaction(SIGWINCH, &sig, nullptr);
  assert(res == 0);

  // Destroy the self-pipe.
  assert(resize_pipe_[0] != -1);
  assert(resize_pipe_[1] != -1);

  int close_res = close(resize_pipe_[0]);
  if (close_res != 0) {
    error_stream << "Closing the self-pipe for screen resizing failed: " <<
      std::strerror(errno) << '\n';
    res = 1;
  }
  resize_pipe_[0] = -1;

  close_res = close(resize_pipe_[1]);
  if (close_res != 0) {
    error_stream << "Closing the self-pipe for screen resizing failed: " <<
      std::strerror(errno) << '\n';
    res = 1;
  }
  resize_pipe_[1] = -1;

  return res;
}

void TestApp::sigwinch_handler(int signum)
{
  assert(signum == SIGWINCH);

  if (resize_pending_)
    return;

  int saved_errno = errno;
  int res = write(resize_pipe_[1], "@", 1);
  errno = saved_errno;
  if (res == 1) {
    resize_pending_ = true;
    return;
  }

  // Cannot reasonably recover from this error. This should be absolutely rare.
  char write_error[] = "Write to the self-pipe for screen resizing failed.\n";
  write(STDERR_FILENO, write_error, sizeof(write_error));
  _exit(13);
}

TestApp::TestApp() : exit_now_(false), resize_pending_(false)
{
  resize_pipe_[0] = -1;
  resize_pipe_[1] = -1;
}

int TestApp::runAll()
{
  int res = 1;
  CppConsUI::Error error;
  bool cppconsui_interface_initialized = false;
  bool cppconsui_input_initialized = false;
  bool cppconsui_output_initialized = false;
  bool screen_resizing_initialized = false;
  pollfd watch[2];
  std::ostringstream error_stream;
  int timeout;

  // Initialize locale support.
  setlocale(LC_ALL, "");

  // Initialize CppConsUI.
  CppConsUI::AppInterface interface = {sigc::mem_fun(this, &TestApp::redraw),
    sigc::mem_fun(this, &TestApp::logDebug)};
  CppConsUI::initializeConsUI(interface);
  cppconsui_interface_initialized = true;

  // Initialize CppConsUI input and output.
  if (COREMANAGER->initializeInput(error) != 0) {
    error_stream << error.getString() << '\n';
    goto out;
  }
  cppconsui_input_initialized = true;
  if (COREMANAGER->initializeOutput(error) != 0) {
    error_stream << error.getString() << '\n';
    goto out;
  }
  cppconsui_output_initialized = true;

  // Declare local bindables.
  declareBindable("testapp", "quit", sigc::mem_fun(this, &TestApp::quit),
    InputProcessor::BINDABLE_OVERRIDE);

  // Set up key binds.
  KEYCONFIG->loadDefaultKeyConfig();
  KEYCONFIG->bindKey("testapp", "quit", "F10");

  // Set up screen resizing.
  if (initializeScreenResizing(error_stream) != 0)
    goto out;
  screen_resizing_initialized = true;

  // Set up the actual test.
  setupTest();

  COREMANAGER->setTopInputProcessor(*this);

  // Run the main loop.
  watch[0].fd = STDIN_FILENO;
  watch[0].events = POLLIN;
  watch[1].fd = resize_pipe_[0];
  watch[1].events = POLLIN;
  timeout = -1;

  while (!exit_now_) {
    if (COREMANAGER->isRedrawPending()) {
      if (COREMANAGER->draw(error) != 0) {
        error_stream << error.getString() << '\n';
        goto out;
      }
    }

    // Wait for an event.
    int poll_res;
    struct timespec timeout_ts, current_ts;
    do {
      int remaining_timeout = -1;

      if (timeout >= 0) {
        // Calculate remaining timeout.
        clock_gettime(CLOCK_MONOTONIC, &current_ts);
        unsigned long tdiff = (current_ts.tv_sec - timeout_ts.tv_sec) * 1000 +
          current_ts.tv_nsec / 1000000 - timeout_ts.tv_nsec / 1000000;

        if (tdiff >= static_cast<unsigned>(timeout))
          remaining_timeout = 0;
        else
          remaining_timeout = timeout - tdiff;
      }

      poll_res = poll(watch, sizeof(watch) / sizeof(watch[0]),
        remaining_timeout);
    } while (poll_res == -1 && errno == EINTR);

    if (poll_res > 0) {
      CppConsUI::Error error;

      if ((watch[0].revents & POLLIN) != 0) {
        // Stdin bytes available.
        if (COREMANAGER->processStandardInput(&timeout, error) != 0) {
          error_stream << error.getString() << '\n';
          goto out;
        }
        if (timeout >= 0) {
          // Remember when this timeout started.
          clock_gettime(CLOCK_MONOTONIC, &timeout_ts);
        }
      }
      if ((watch[1].revents & POLLIN) != 0) {
        // Screen resize done.
        char buf[1024];
        int res = read(resize_pipe_[0], buf, sizeof(buf));
        assert(res > 0);

        resize_pending_ = false;

        if (COREMANAGER->resize(error) != 0) {
          error_stream << error.getString() << '\n';
          goto out;
        }
      }
    }
    else if (poll_res == 0) {
      // Timeout reached.
      timeout = -1;
      if (COREMANAGER->processStandardInputTimeout(error) != 0) {
        error_stream << error.getString() << '\n';
        goto out;
      }
    }
    else {
      // An error occurred.
      error_stream << "Main loop error: " << std::strerror(errno) << '\n';
      goto out;
    }
  }

  // Everything went ok.
  res = 0;

out:
  // Finalize screen resizing.
  if (screen_resizing_initialized && finalizeScreenResizing(error_stream) != 0)
    res = 1;

  // Finalize CppConsUI input and output.
  if (cppconsui_output_initialized && COREMANAGER->finalizeOutput(error) != 0) {
    error_stream << error.getString() << '\n';
    res = 1;
  }
  if (cppconsui_input_initialized && COREMANAGER->finalizeInput(error) != 0) {
    error_stream << error.getString() << '\n';
    res = 1;
  }

  // Finalize CppConsUI interface.
  if (cppconsui_interface_initialized)
    CppConsUI::finalizeConsUI();

  std::cerr << error_stream.str() << std::flush;

  return res;
}

// Main function.
int main()
{
  return TestApp::run();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
