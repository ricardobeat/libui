// 23 april 2015
#include "uipriv_unix.h"

static GArray *menus = NULL;
static gboolean menusFinalized = FALSE;

struct menu {
	uiMenu m;
	char *name;
	GHashTable *gtkMenuItems;		// map[GtkWindow]GtkMenuItem
	GHashTable *gtkSubmenus;		// map[GtkWindow]GtkMenu
	GArray *items;					// []struct menuItem
};

struct menuItem {
	uiMenuItem mi;
	char *name;
	int type;
	void (*onClicked)(uiMenuItem *, uiWindow *, void *);
	void *onClickedData;
	GtkWidget *baseItem;			// for property binding
	GHashTable *uiWindows;			// map[GtkMenuItem]uiWindow
};

enum {
	typeRegular,
	typeCheckbox,
	typeQuit,
	typePreferences,
	typeAbout,
	typeSeparator,
};

#define NEWHASH() g_hash_table_new(g_direct_hash, g_direct_equal)

static uiMenuItem *newItem(struct menu *m, int type, const char *name)
{
	struct menuItem *item;

	if (menusFinalized)
		complain("attempt to create a new menu item after menus have been finalized");

	g_array_set_size(m->items, m->items->len + 1);
	item = &g_array_index(m->items, struct menuItem, m->items->len - 1);

	item->type = type;
	switch (item->type) {
	case typeQuit:
		item->name = g_strdup("Quit");
		break;
	case typePreferences:
		item->name = g_strdup("Preferences...");
		break;
	case typeAbout:
		item->name = g_strdup("About");
		break;
	case typeSeparator:
		break;
	default:
		item->name = g_strdup(name);
		break;
	}

	switch (item->type) {
	case typeCheckbox:
		item->baseItem = gtk_check_menu_item_new_with_label(item->name);
		break;
	case typeSeparator:
		item->baseItem = gtk_separator_menu_item_new();
		break;
	default:
		item->baseItem = gtk_menu_item_new_with_label(item->name);
		break;
	}

	item->uiWindows = NEWHASH();

	// TODO vtable

	return uiMenuItem(item);
}

uiMenuItem *menuAppendItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeRegular, name);
}

uiMenuItem *menuAppendCheckItem(uiMenu *mm, const char *name)
{
	return newItem((struct menu *) mm, typeCheckbox, name);
}

uiMenuItem *menuAppendQuitItem(uiMenu *mm)
{
	// TODO check multiple quit items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeQuit, NULL);
}

uiMenuItem *menuAppendPreferencesItem(uiMenu *mm)
{
	// TODO check multiple preferences items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typePreferences, NULL);
}

uiMenuItem *menuAppendAboutItem(uiMenu *mm)
{
	// TODO check multiple about items
	// TODO conditionally add separator
	newItem((struct menu *) mm, typeSeparator, NULL);
	return newItem((struct menu *) mm, typeAbout, NULL);
}

void menuAppendSeparator(uiMenu *mm)
{
	// TODO check multiple about items
	newItem((struct menu *) mm, typeSeparator, NULL);
}

uiMenu *uiNewMenu(const char *name)
{
	struct menu *m;

	if (menusFinalized)
		complain("attempt to create a new menu after menus have been finalized");
	if (menus == NULL)
		menus = g_array_new(FALSE, TRUE, sizeof (struct menu));

	// thanks Company in irc.gimp.net/#gtk+
	g_array_set_size(menus, menus->len + 1);
	m = &g_array_index(menus, struct menu, menus->len - 1);

	m->name = g_strdup(name);
	m->gtkMenuItems = NEWHASH();
	m->gtkSubmenus = NEWHASH();
	m->items = g_array_new(FALSE, TRUE, sizeof (struct menuItem));

	uiMenu(m)->AppendItem = menuAppendItem;
	uiMenu(m)->AppendCheckItem = menuAppendCheckItem;
	uiMenu(m)->AppendQuitItem = menuAppendQuitItem;
	uiMenu(m)->AppendPreferencesItem = menuAppendPreferencesItem;
	uiMenu(m)->AppendAboutItem = menuAppendAboutItem;
	uiMenu(m)->AppendSeparator = menuAppendSeparator;

	return uiMenu(m);
}

/*
void menuItemDestroy(struct menuItem *item)
{
	// TODO checck that item->uiWindows is empty
	g_hash_table_destroy(item->uiWindows);
	gtk_widget_destroy(item->baseItem);
	if (item->name != NULL)
		g_free(item->name);
}

void menuDestroy(void)
{
	guint i, j;
	struct menu *m;

	for (i = 0; i < menus->len; i++) {
		m = &g_array_index(menus, struct menu, i);
		for (j = 0; j < m->items->len; j++)
			menuItemDestroy(&g_array_index(m->items, struct menuItem, j));
		g_array_free(m->items, TRUE);
		// TODO check that m->gtkSubmenus is empty
		g_hash_table_destroy(m->gtkSubmenus);
		// TODO check that m->gtkMenuItem is empty
		g_hash_table_destroy(m->gtkMenuItem);
		g_free(m->name);
	}
	g_array_free(menus, TRUE);
}
*/