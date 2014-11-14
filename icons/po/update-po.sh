#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f kapow.desktop.in.h
rm -f kapow.desktop.in
cp kapow.desktop kapow.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e 's/^Name/_Name/' \
	-e 's/^GenericName/_GenericName/' \
	-e 's/^Comment/_Comment/' \
	-i kapow.desktop.in
intltool-extract --quiet --type=gettext/ini kapow.desktop.in

rm -f kapow.appdata.xml.in.h
rm -f kapow.appdata.xml.in
cp kapow.appdata.xml kapow.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e 's/<p>/<_p>/' \
	-e 's/<\/p>/<\/_p>/' \
	-e 's/<summary>/<_summary>/' \
	-e 's/<\/summary>/<\/_summary>/' \
	-e 's/<name>/<_name>/' \
	-e 's/<\/name>/<\/_name>/' \
	-i kapow.appdata.xml.in
intltool-extract --quiet --type=gettext/xml kapow.appdata.xml.in

cd po
echo ' DONE'


echo -n 'Extracting messages...'
xgettext --from-code=UTF-8 --c++ --keyword=_ --keyword=N_:1 \
	--package-name='Kapow' --copyright-holder='Graeme Gott' \
	--output=description.pot ../*.h
sed 's/CHARSET/UTF-8/' -i description.pot
echo ' DONE'


echo -n 'Updating translations...'
for POFILE in *.po;
do
	echo -n " $POFILE"
	msgmerge --quiet --update --backup=none $POFILE description.pot
done
echo ' DONE'


echo -n 'Merging translations...'
cd ..

intltool-merge --quiet --desktop-style po kapow.desktop.in kapow.desktop
rm -f kapow.desktop.in.h
rm -f kapow.desktop.in

intltool-merge --quiet --xml-style po kapow.appdata.xml.in kapow.appdata.xml
rm -f kapow.appdata.xml.in.h
rm -f kapow.appdata.xml.in

echo ' DONE'
