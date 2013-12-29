/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qtimer.h>
#include <qtooltip.h>

#include <kaction.h>
#include <kapplication.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "configdialog.h"

#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"

#define OUTPUT_CONNECTED		(1 << 0)
#define OUTPUT_UNKNOWN			(1 << 1)
#define OUTPUT_DISCONNECTED		(1 << 2)
#define OUTPUT_ON			(1 << 3)
#define OUTPUT_ALL			(0xf)

	KRandRSystemTray::KRandRSystemTray(QWidget* parent, const char *name)
	: KSystemTray(parent, name)
	, m_popupUp(false)
	, m_help(new KHelpMenu(this, KGlobal::instance()->aboutData(), false, actionCollection()))
{
	setPixmap(KSystemTray::loadIcon("randr"));
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	connect(this, SIGNAL(quitSelected()), kapp, SLOT(quit()));
	QToolTip::add(this, i18n("Screen resize & rotate"));
	my_parent = parent;

	printf("Reading configuration...\n\r");
	globalKeys = new KGlobalAccel(this);
	KGlobalAccel* keys = globalKeys;
#include "krandrbindings.cpp"
	// the keys need to be read from kdeglobals, not kickerrc
	globalKeys->readSettings();
	globalKeys->setEnabled(true);
	globalKeys->updateConnections();

	connect(kapp, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));

	randr_display = XOpenDisplay(NULL);
}

void KRandRSystemTray::mousePressEvent(QMouseEvent* e)
{
	// Popup the context menu with left-click
	if (e->button() == LeftButton) {
		contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
		e->accept();
		return;
	}

	KSystemTray::mousePressEvent(e);
}

void KRandRSystemTray::contextMenuAboutToShow(KPopupMenu* menu)
{
	// Reload the randr configuration...
	XRROutputInfo *output_info;
	char *output_name;
	RROutput output_id;
	int i;
	int lastIndex = 0;
	int screenDeactivated = 0;

	if (isValid() == true) {
		randr_screen_info = read_screen_info(randr_display);

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for ON outputs
			if (!randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}
			if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
				continue;
			}

			output_name = output_info->name;
			output_id = randr_screen_info->outputs[i]->id;

			// Deactivate this display to avoid a crash!
			randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[i];
			randr_screen_info->cur_output->auto_set = 0;
			randr_screen_info->cur_output->off_set = 1;
			output_off(randr_screen_info, randr_screen_info->cur_output);
			main_low_apply(randr_screen_info);

			screenDeactivated = 1;
		}

		if (screenDeactivated == 1) {
			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
	}

	menu->clear();
	menu->setCheckable(true);

	if (!isValid()) {
		lastIndex = menu->insertItem(i18n("Required X Extension Not Available"));
		menu->setItemEnabled(lastIndex, false);

	} else {
		m_screenPopups.clear();
		for (int s = 0; s < numScreens() /*&& numScreens() > 1 */; s++) {
			setCurrentScreen(s);
			if (s == screenIndexOfWidget(this)) {
				/*lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);*/
			} else {
				KPopupMenu* subMenu = new KPopupMenu(menu, QString("screen%1").arg(s+1).latin1());
				m_screenPopups.append(subMenu);
				populateMenu(subMenu);
				lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1), subMenu);
				connect(subMenu, SIGNAL(activated(int)), SLOT(slotScreenActivated()));
			}
		}

		setCurrentScreen(screenIndexOfWidget(this));
		populateMenu(menu);
	}

	addOutputMenu(menu);

	menu->insertTitle(SmallIcon("randr"), i18n("Global Configuation"));

// 	KAction *actPrefs = new KAction( i18n( "Configure Display..." ),
// 		SmallIconSet( "configure" ), KShortcut(), this, SLOT( slotPrefs() ),
// 		actionCollection() );
// 	actPrefs->plug( menu );

	KAction *actSKeys = new KAction( i18n( "Configure Shortcut Keys..." ),
		SmallIconSet( "configure" ), KShortcut(), this, SLOT( slotSKeys() ),
		actionCollection() );
	actSKeys->plug( menu );

	menu->insertItem(SmallIcon("help"),KStdGuiItem::help().text(), m_help->menu());
	KAction *quitAction = actionCollection()->action(KStdAction::name(KStdAction::Quit));
	quitAction->plug(menu);
}

void KRandRSystemTray::slotScreenActivated()
{
	setCurrentScreen(m_screenPopups.find(static_cast<const KPopupMenu*>(sender())));
}

void KRandRSystemTray::configChanged()
{
	refresh();

	static bool first = true;

	if (!first)
		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		currentScreen()->changedMessage(), SmallIcon("window_fullscreen"),
		this, "ScreenChangeNotification");

	first = false;
}

int KRandRSystemTray::GetDefaultResolutionParameter()
{
	int returnIndex = 0;

	int numSizes = currentScreen()->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = currentScreen()->pixelCount(i);
	}

	int highest = -1, highestIndex = -1;

	for (int i = 0; i < numSizes; i++) {
		if (sizeSort[i] && sizeSort[i] > highest) {
			highest = sizeSort[i];
			highestIndex = i;
		}
	}
	sizeSort[highestIndex] = -1;
	Q_ASSERT(highestIndex != -1);

	returnIndex = highestIndex;

	delete [] sizeSort;
	sizeSort = 0L;

	return returnIndex;
}

void KRandRSystemTray::populateMenu(KPopupMenu* menu)
{
	int lastIndex = 0;

	menu->insertTitle(SmallIcon("window_fullscreen"), i18n("Screen Size"));

	int numSizes = currentScreen()->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = currentScreen()->pixelCount(i);
	}

	for (int j = 0; j < numSizes; j++) {
		int highest = -1, highestIndex = -1;

		for (int i = 0; i < numSizes; i++) {
			if (sizeSort[i] && sizeSort[i] > highest) {
				highest = sizeSort[i];
				highestIndex = i;
			}
		}
		sizeSort[highestIndex] = -1;
		Q_ASSERT(highestIndex != -1);

		lastIndex = menu->insertItem(i18n("%1 x %2").arg(currentScreen()->pixelSize(highestIndex).width()).arg(currentScreen()->pixelSize(highestIndex).height()));

		if (currentScreen()->proposedSize() == highestIndex)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, highestIndex);
		menu->connectItem(lastIndex, this, SLOT(slotResolutionChanged(int)));
	}
	delete [] sizeSort;
	sizeSort = 0L;

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (currentScreen()->rotations() != RandRScreen::Rotate0) {
		menu->insertTitle(SmallIcon("reload"), i18n("Orientation"));

		for (int i = 0; i < 6; i++) {
			if ((1 << i) & currentScreen()->rotations()) {
				lastIndex = menu->insertItem(currentScreen()->rotationIcon(1 << i), RandRScreen::rotationName(1 << i));

				if (currentScreen()->proposedRotation() & (1 << i))
					menu->setItemChecked(lastIndex, true);

				menu->setItemParameter(lastIndex, 1 << i);
				menu->connectItem(lastIndex, this, SLOT(slotOrientationChanged(int)));
			}
		}
	}

	QStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	if (rr.count())
		menu->insertTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); ++it, i++) {
		lastIndex = menu->insertItem(*it);

		if (currentScreen()->proposedRefreshRate() == i)
			menu->setItemChecked(lastIndex, true);

		menu->setItemParameter(lastIndex, i);
		menu->connectItem(lastIndex, this, SLOT(slotRefreshRateChanged(int)));
	}
}

void KRandRSystemTray::slotResolutionChanged(int parameter)
{
	if (currentScreen()->currentSize() == parameter) {
		//printf("This resolution is already in use; applying again...\n\r");
		currentScreen()->proposeSize(parameter);
		currentScreen()->applyProposed();
		return;
	}

	currentScreen()->proposeSize(parameter);

	currentScreen()->proposeRefreshRate(-1);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotOrientationChanged(int parameter)
{
	int propose = currentScreen()->currentRotation();

	if (parameter & RandRScreen::RotateMask)
		propose &= RandRScreen::ReflectMask;

	propose ^= parameter;

	if (currentScreen()->currentRotation() == propose)
		return;

	currentScreen()->proposeRotation(propose);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotRefreshRateChanged(int parameter)
{
	if (currentScreen()->currentRefreshRate() == parameter)
		return;

	currentScreen()->proposeRefreshRate(parameter);

	if (currentScreen()->applyProposedAndConfirm()) {
		KConfig config("kcmrandrrc");
		if (syncTrayApp(config))
			currentScreen()->save(config);
	}
}

void KRandRSystemTray::slotPrefs()
{
	KCMultiDialog *kcm = new KCMultiDialog( KDialogBase::Plain, i18n( "Configure" ), this );

	kcm->addModule( "displayconfig" );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->exec();
}

void KRandRSystemTray::slotSettingsChanged(int category)
{
	if ( category == (int) KApplication::SETTINGS_SHORTCUTS ) {
		globalKeys->readSettings();
		globalKeys->updateConnections();
	}
}

void KRandRSystemTray::slotSKeys()
{
	ConfigDialog *dlg = new ConfigDialog(globalKeys, true);

	if ( dlg->exec() == QDialog::Accepted ) {
		dlg->commitShortcuts();
		globalKeys->writeSettings(0, true);
		globalKeys->updateConnections();
	}

	delete dlg;
}

void KRandRSystemTray::slotCycleDisplays()
{
	XRROutputInfo *output_info;
	char *output_name;
	RROutput output_id;
	int i;
	int lastIndex = 0;
	int current_on_index = -1;
	int max_index = -1;
	int prev_on_index;
	Status s;

	randr_screen_info = read_screen_info(randr_display);

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ON outputs...
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}
		// ...that are connected
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}

		output_name = output_info->name;
		output_id = randr_screen_info->outputs[i]->id;
		current_on_index = i;
		if (i > max_index) {
			max_index = i;
		}
	}

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for CONNECTED outputs....
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}
		// ...that are not ON
		if (randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		output_name = output_info->name;
		output_id = randr_screen_info->outputs[i]->id;
		if (i > max_index) {
			max_index = i;
		}
	}

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ALL outputs that are not connected....
		if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
			continue;
		}
		// ...or ON
		if (randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		output_name = output_info->name;
		output_id = randr_screen_info->outputs[i]->id;
		if (i > max_index) {
			max_index = i;
		}
	}

	printf("Active: %d\n\r", current_on_index);
	printf("Max: %d\n\r", max_index);

	if ((current_on_index == -1) && (max_index == -1)) {
		// There is no connected display available!  ABORT
		return;
	}

	prev_on_index = current_on_index;
	current_on_index = current_on_index + 1;
	if (current_on_index > max_index) {
		current_on_index = 0;
	}
	while (RR_Disconnected == randr_screen_info->outputs[current_on_index]->info->connection) {
		current_on_index = current_on_index + 1;
		if (current_on_index > max_index) {
			current_on_index = 0;
		}
	}
	if (prev_on_index != current_on_index) {
		randr_screen_info->cur_crtc = randr_screen_info->outputs[current_on_index]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[current_on_index];
		randr_screen_info->cur_output->auto_set = 1;
		randr_screen_info->cur_output->off_set = 0;
		output_auto (randr_screen_info, randr_screen_info->cur_output);
		i=main_low_apply(randr_screen_info);

		if (randr_screen_info->outputs[current_on_index]->cur_crtc) {
			if (prev_on_index != -1) {
				if (randr_screen_info->outputs[prev_on_index]->cur_crtc != NULL) {
					if (RR_Disconnected != randr_screen_info->outputs[prev_on_index]->info->connection) {
						randr_screen_info->cur_crtc = randr_screen_info->outputs[prev_on_index]->cur_crtc;
						randr_screen_info->cur_output = randr_screen_info->outputs[prev_on_index];
						randr_screen_info->cur_output->auto_set = 0;
						randr_screen_info->cur_output->off_set = 1;
						output_off(randr_screen_info, randr_screen_info->cur_output);
						i=main_low_apply(randr_screen_info);
					}
				}
			}

			// Do something about the disconnected outputs
			for (i = 0; i < randr_screen_info->n_output; i++) {
				output_info = randr_screen_info->outputs[i]->info;
				// Look for ON outputs
				if (!randr_screen_info->outputs[i]->cur_crtc) {
					continue;
				}
				if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
					continue;
				}

				output_name = output_info->name;
				output_id = randr_screen_info->outputs[i]->id;

				// Deactivate this display to avoid a crash!
				randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
				randr_screen_info->cur_output = randr_screen_info->outputs[i];
				randr_screen_info->cur_output->auto_set = 0;
				randr_screen_info->cur_output->off_set = 1;
				output_off(randr_screen_info, randr_screen_info->cur_output);
				main_low_apply(randr_screen_info);
			}

			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
		else {
			output_name = randr_screen_info->outputs[current_on_index]->info->name;
			KMessageBox::sorry(my_parent, i18n("<b>Unable to activate output %1</b><p>Either the output is not connected to a display,<br>or the display configuration is not detectable").arg(output_name), i18n("Output Unavailable"));
		}
	}
}

void KRandRSystemTray::findPrimaryDisplay()
{
	XRROutputInfo *output_info;
	char *output_name;
	RROutput output_id;
	int i;
	int lastIndex = 0;

	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ON outputs...
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		// ...that are connected
		if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
			continue;
		}

		output_name = output_info->name;
		output_id = randr_screen_info->outputs[i]->id;
		printf("ACTIVE CHECK: Found output %s\n\r", output_name);

		randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[i];
	}
}

void KRandRSystemTray::addOutputMenu(KPopupMenu* menu)
{
	XRROutputInfo *output_info;
	char *output_name;
	RROutput output_id;
	int i;
	int lastIndex = 0;
	int connected_displays = 0;

	if (isValid() == true) {
		menu->insertTitle(SmallIcon("kcmkwm"), i18n("Output Port"));

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for ON outputs
			if (!randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				continue;
			}

			output_name = output_info->name;
			output_id = randr_screen_info->outputs[i]->id;
			//printf("ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Active)").arg(output_name));
			menu->setItemChecked(lastIndex, true);
			menu->connectItem(lastIndex, this, SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);

			connected_displays++;
		}

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for CONNECTED outputs....
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				continue;
			}
			// ...that are not ON
			if (randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}

			output_name = output_info->name;
			output_id = randr_screen_info->outputs[i]->id;
			//printf("CONNECTED, NOT ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Connected, Inactive)").arg(output_name));
			menu->setItemChecked(lastIndex, false);
			menu->connectItem(lastIndex, this, SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);

			connected_displays++;
		}

		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			// Look for ALL outputs that are not connected....
			if (RR_Disconnected != randr_screen_info->outputs[i]->info->connection) {
				continue;
			}
			// ...or ON
			if (randr_screen_info->outputs[i]->cur_crtc) {
				continue;
			}

			output_name = output_info->name;
			output_id = randr_screen_info->outputs[i]->id;
			//printf("DISCONNECTED, NOT ON: Found output %s\n\r", output_name);

			lastIndex = menu->insertItem(i18n("%1 (Disconnected, Inactive)").arg(output_name));
			menu->setItemChecked(lastIndex, false);
			menu->setItemEnabled(lastIndex, false);
			menu->connectItem(lastIndex, this, SLOT(slotOutputChanged(int)));
			menu->setItemParameter(lastIndex, i);
		}

		lastIndex = menu->insertItem(SmallIcon("forward"), i18n("Next available output"));
		if (connected_displays < 2) {
			menu->setItemEnabled(lastIndex, false);
		}
		menu->connectItem(lastIndex, this, SLOT(slotCycleDisplays()));
	}
}

void KRandRSystemTray::slotOutputChanged(int parameter)
{
	XRROutputInfo *output_info;
	char *output_name;
	RROutput output_id;
	int i;
	Status s;
	int num_outputs_on;

	num_outputs_on = 0;
	for (i = 0; i < randr_screen_info->n_output; i++) {
		output_info = randr_screen_info->outputs[i]->info;
		// Look for ON outputs
		if (!randr_screen_info->outputs[i]->cur_crtc) {
			continue;
		}

		num_outputs_on++;
	}

	if (!randr_screen_info->outputs[parameter]->cur_crtc) {
		//printf("Screen was off, turning it on...\n\r");

		randr_screen_info->cur_crtc = randr_screen_info->outputs[parameter]->cur_crtc;
		randr_screen_info->cur_output = randr_screen_info->outputs[parameter];
		randr_screen_info->cur_output->auto_set = 1;
		randr_screen_info->cur_output->off_set = 0;
		output_auto (randr_screen_info, randr_screen_info->cur_output);
		i=main_low_apply(randr_screen_info);

		if (!randr_screen_info->outputs[parameter]->cur_crtc) {
			output_name = randr_screen_info->outputs[parameter]->info->name;
			KMessageBox::sorry(my_parent, i18n("<b>Unable to activate output %1</b><p>Either the output is not connected to a display,<br>or the display configuration is not detectable").arg(output_name), i18n("Output Unavailable"));
		}
	}
	else {
		if (num_outputs_on > 1) {
			//printf("Screen was on, turning it off...\n\r");
			randr_screen_info->cur_crtc = randr_screen_info->outputs[parameter]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[parameter];
			randr_screen_info->cur_output->auto_set = 0;
			randr_screen_info->cur_output->off_set = 1;
			output_off(randr_screen_info, randr_screen_info->cur_output);
			i=main_low_apply(randr_screen_info);

			findPrimaryDisplay();
			refresh();

			currentScreen()->proposeSize(GetDefaultResolutionParameter());
			currentScreen()->applyProposed();
		}
		else {
			KMessageBox::sorry(my_parent, i18n("<b>You are attempting to deactivate the only active output</b><p>You must keep at least one display output active at all times!"), i18n("Invalid Operation Requested"));
		}
	}
}
