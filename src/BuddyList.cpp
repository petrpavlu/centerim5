/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010 by CenterIM developers
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

#include "BuddyList.h"

#include "Conf.h"
#include "CenterIM.h"
#include "Log.h"

#include <cppconsui/Keys.h>

#include <libpurple/pounce.h>

#include "gettext.h"

BuddyList *BuddyList::Instance()
{
	static BuddyList instance;
	return &instance;
}

BuddyList::BuddyList()
: Window(0, 0, 80, 24)
{
	SetColorScheme("buddylist");

	/* TODO Check if this has been moved to purple_blist_init(). Remove these
	 * lines if it was as this will probably move to purple_init(), the
	 * buddylist object should be available a lot more early and the uiops
	 * should be set a lot more early. (All in all a lot of work.) */
	buddylist = purple_blist_new();
	buddylist->ui_data = this;
	purple_set_blist(buddylist);

	// load the pounces
	purple_pounces_load();

	// setup the callbacks for the buddylist
	memset(&centerim_blist_ui_ops, 0, sizeof(centerim_blist_ui_ops));
	centerim_blist_ui_ops.new_list = new_list_;
	centerim_blist_ui_ops.new_node = new_node_;
	//centerim_blist_ui_ops.show = show_;
	centerim_blist_ui_ops.update = update_;
	centerim_blist_ui_ops.remove = remove_;
	centerim_blist_ui_ops.destroy = destroy_;
	//centerim_blist_ui_ops.set_visible = set_visible_;
	centerim_blist_ui_ops.request_add_buddy = request_add_buddy_;
	centerim_blist_ui_ops.request_add_chat = request_add_chat_;
	centerim_blist_ui_ops.request_add_group = request_add_group_;
	// since 2.6.0
	//centerim_blist_ui_ops.save_node = save_node_;
	//centerim_blist_ui_ops.remove_node = remove_node_;
	//centerim_blist_ui_ops.save_account = save_account_;
	purple_blist_set_ui_ops(&centerim_blist_ui_ops);

	g_timeout_add(0, timeout_once_load, NULL);

	treeview = new TreeView(width - 2, height - 2);
	AddWidget(*treeview, 0, 0);

	MoveResizeRect(CONF->GetBuddyListDimensions());
}

gboolean BuddyList::timeout_once_load(gpointer data)
{
	// loads the buddy list from ~/.centerim5/blist.xml
	purple_blist_load();

	return FALSE;
}

void BuddyList::Close()
{
}

void BuddyList::new_list(PurpleBuddyList *list)
{
	if (buddylist != list)
		LOG->Write(Log::Level_error, "Different Buddylist detected!\n");
}

void BuddyList::new_node(PurpleBlistNode *node)
{
	g_assert(!node->ui_data);

	BuddyListNode *bnode;

	node->ui_data = bnode = BuddyListNode::CreateNode(node);

	BuddyListNode *parent = bnode->GetParentNode();
	bnode->ref = treeview->AddNode(parent ? parent->ref : treeview->Root(), *bnode);
	treeview->Collapse(bnode->ref);

	if (PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		treeview->SetStyle(bnode->ref, TreeView::STYLE_VOID);

		/* The core seems to expect the UI to add the buddies (according
		 * to finch). */
		for (node = node->child; node; node = node->next)
			new_node(node);
	}
}

void BuddyList::update(PurpleBuddyList *list, PurpleBlistNode *node)
{
	BuddyListNode *bnode = (BuddyListNode *) node->ui_data;
	g_assert(bnode);

	// update the node data
	BuddyListNode *parent = bnode->GetParentNode();
	// the parent could have changed, so re-parent the node
	if (parent)
		treeview->SetParent(bnode->ref, parent->ref);

	bnode->Update();

	/* If the node is a buddy, we also have to update the contact this buddy
	 * belongs to. */
	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		update(list, node->parent);
		/* TODO Is it possible for a node's parent not to be a contact? If so,
		 * then this call could use some checks. */
	}
}

void BuddyList::remove(PurpleBuddyList *list, PurpleBlistNode *node)
{
	// XXX how does this happen?
	g_return_if_fail(node->ui_data);

	BuddyListNode *bnode = (BuddyListNode*)node->ui_data;

	// TODO check for subnodes (if this is a group for instance)
	treeview->DeleteNode(bnode->ref, false);
}

void BuddyList::destroy(PurpleBuddyList *list)
{
}

void BuddyList::request_add_buddy(PurpleAccount *account, const char *username, const char *group, const char *alias)
{
	AddBuddyWindow *bud = new AddBuddyWindow(account, username, group, alias);
	bud->Show();
}

void BuddyList::MoveResize(int newx, int newy, int neww, int newh)
{
	Window::MoveResize(newx, newy, neww, newh);

	treeview->MoveResize(0, 0, neww - 2, newh - 2);
}

void BuddyList::ScreenResized()
{
	MoveResizeRect(CENTERIM->ScreenAreaSize(CenterIM::BuddyListArea));
}

BuddyList::AccountsBox::AccountsBox(PurpleAccount *account)
: selected(account)
{
	GList *i = purple_accounts_get_all();

	if (!selected && i)
		selected = (PurpleAccount *) i->data;

	for ( ; i; i = i->next) {
		PurpleAccount *account = (PurpleAccount *) i->data;
		gchar *label = g_strdup_printf("[%s] %s",
				purple_account_get_protocol_name(account),
				purple_account_get_username(account));
		AddOption(label, account);
		g_free(label);
	}

	UpdateText();

	signal_selection_changed.connect(
			sigc::mem_fun(this, &BuddyList::AccountsBox::OnAccountChanged));
}

void BuddyList::AccountsBox::UpdateText()
{
	gchar *label = g_strdup_printf("%s: [%s] %s", _("Account"),
			purple_account_get_protocol_name(selected),
			purple_account_get_username(selected));
	SetText(label);
	g_free(label);
}

void BuddyList::AccountsBox::OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry)
{
	selected = (PurpleAccount *) new_entry.data;
	UpdateText();
}

BuddyList::NameButton::NameButton(bool alias, const gchar *val)
: Button("")
, dialog(NULL)
{
	if (alias)
		text = _("Alias");
	else
		text = _("Buddy name");

	// value always points to an allocated string
	if (val)
		value = g_strdup(value);
	else
		value = g_strdup("");

	UpdateText();

	signal_activate.connect(sigc::mem_fun(this,
				&BuddyList::NameButton::OnActivate));
}

BuddyList::NameButton::~NameButton()
{
	g_free(value);
}

void BuddyList::NameButton::UpdateText()
{
	gchar *label = g_strdup_printf("%s: %s", text, value);
	SetText(label);
	g_free(label);
}

void BuddyList::NameButton::OnActivate()
{
	g_assert(!dialog);

	dialog = new InputDialog(text, value);
	dialog->signal_response.connect(
			sigc::mem_fun(this, &BuddyList::NameButton::ResponseHandler));
	dialog->Show();
}

void BuddyList::NameButton::ResponseHandler(Dialog::ResponseType response)
{
	g_assert(dialog);

	switch (response) {
		case Dialog::ResponseOK:
			g_free(value);
			value = g_strdup(dialog->GetText());

			UpdateText();
			break;
		default:
			break;
	}
	dialog = NULL;
}

BuddyList::GroupBox::GroupBox(const gchar *group)
: selected(NULL)
{
	PurpleBlistNode *i = purple_blist_get_root();

	while (i) {
		if (PURPLE_BLIST_NODE_IS_GROUP(i)) {
			if (!selected)
				selected = g_strdup(purple_group_get_name((PurpleGroup *)(i)));
			AddOption(purple_group_get_name((PurpleGroup *)(i)));
		}
		i = i->next;
	}
	if (!selected) {
		AddOption(_("Buddies"));
		selected = g_strdup(_("Buddies"));
	}

	UpdateText();

	signal_selection_changed.connect(
			sigc::mem_fun(this, &BuddyList::GroupBox::OnAccountChanged));
}

BuddyList::GroupBox::~GroupBox()
{
	g_free(selected);
}

void BuddyList::GroupBox::UpdateText()
{
	gchar *label = g_strdup_printf("%s: %s", _("Group"), selected);
	SetText(label);
	g_free(label);
}

void BuddyList::GroupBox::OnAccountChanged(const ComboBox::ComboBoxEntry& new_entry)
{
	g_free(selected);
	selected = g_strdup(new_entry.GetText());
	UpdateText();
}

BuddyList::AddBuddyWindow::AddBuddyWindow(PurpleAccount *account,
		const char *username, const char *group, const char *alias)
: Window(0, 0, 50, 10)
{
	accounts_box = new AccountsBox(account);
	name_button = new NameButton(false, username);
	alias_button = new NameButton(true, alias);
	group_box = new GroupBox(group);
	line = new HorizontalLine(width);

	menu = new HorizontalListBox(width, 1);
	//menu->FocusCycle(Container::FocusCycleLocal);
	menu->AppendItem(_("Add"), sigc::mem_fun(this, &BuddyList::AddBuddyWindow::Add));

	AddWidget(*accounts_box, 0, 0);
	AddWidget(*name_button, 0, 1);
	AddWidget(*alias_button, 0, 2);
	AddWidget(*group_box, 0, 3);
	AddWidget(*line, 0, 4);
	AddWidget(*menu, 0, 5);
}

void BuddyList::AddBuddyWindow::Add()
{
	PurpleAccount *account = accounts_box->GetSelected();
	const char *who = name_button->GetValue();
	const char *whoalias = alias_button->GetValue();
	const char *grp = group_box->GetSelected();

	PurpleGroup *g = purple_find_group(grp);
	PurpleBuddy *b = purple_find_buddy_in_group(account, who, g);

	if (!b) {
		b = purple_buddy_new(account, who, whoalias[0] != '\0' ? whoalias : NULL);
		purple_blist_add_buddy(b, NULL, g, NULL);
		purple_account_add_buddy(account, b);
	}

	Close();
}
