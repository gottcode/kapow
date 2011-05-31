#!/bin/bash

APP='Kapow'
BUNDLE="$APP.app"
VERSION='1.3.3'

# Create disk folder
echo -n 'Copying application bundle... '
rm -f "${APP}_$VERSION.dmg"
rm -rf "$APP"
mkdir "$APP"
cp -pf COPYING "$APP/COPYING.txt"
cp -pf CREDITS "$APP/CREDITS.txt"
cp -Rpf "$BUNDLE" "$APP/"
echo 'Done'

# Copy translations
echo -n 'Copying translations... '
TRANSLATIONS="$APP/$BUNDLE/Contents/Resources/translations"
mkdir "$TRANSLATIONS"
cp -Rf translations/*.qm "$TRANSLATIONS"
echo 'Done'

# Copy Qt translations
echo -n 'Copying Qt translations... '
for translation in $(ls translations | grep qm | cut -d'.' -f1 | cut -d'_' -f2-)
do
	LPROJ="$APP/$BUNDLE/Contents/Resources/${translation}.lproj"
	mkdir "$LPROJ"
	sed "s/????/${translation}/" < locversion.plist > "${LPROJ}/locversion.plist"

	QT_TRANSLATION="/Developer/Applications/Qt/translations/qt_${translation}.qm"
	if [ -e "$QT_TRANSLATION" ]; then
		cp -f "$QT_TRANSLATION" "$TRANSLATIONS"
	fi

	QT_TRANSLATION="/Developer/Applications/Qt/translations/qt_${translation:0:2}.qm"
	if [ -e "$QT_TRANSLATION" ]; then
		cp -f "$QT_TRANSLATION" "$TRANSLATIONS"
	fi
done
echo 'Done'

# Copy frameworks and plugins
echo -n 'Copying frameworks and plugins... '
macdeployqt "$APP/$BUNDLE" -no-plugins
echo 'Done'

# Create disk image
echo -n 'Creating disk image... '
hdiutil create -quiet -ov -srcfolder "$APP" -format UDBZ -volname "$APP" "${APP}_$VERSION.dmg"
hdiutil internet-enable -quiet -yes "${APP}_$VERSION.dmg"
echo 'Done'

# Clean up disk folder
echo -n 'Cleaning up... '
rm -rf "$APP"
echo 'Done'
