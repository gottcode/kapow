#!/bin/bash

APP='Kapow'
BUNDLE="$APP.app"
VERSION='1.3.3'

# Create disk folder
echo -n 'Copying application bundle... '
rm -f "${APP}_$VERSION.dmg"
rm -rf "$APP"
mkdir "$APP"
cp -pf COPYING "$APP/"
cp -Rpf "$BUNDLE" "$APP/"
echo 'Done'

# Copy translations
echo -n 'Copying translations... '
TRANSLATIONS=$APP/$BUNDLE/Contents/Resources/translations
mkdir $TRANSLATIONS
cp -Rf translations/*.qm $TRANSLATIONS
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
