#include "themes.h"
#include <qpalette.h>


ThemeManager::ThemeManager(QObject *parent)
	: QObject{parent}
{
	// Set dark palette coloring
	this->darkPalette = new QPalette();
	this->darkPalette->setColor(QPalette::Window, QColor(53, 53, 53));
	this->darkPalette->setColor(QPalette::WindowText, Qt::white);
	this->darkPalette->setColor(QPalette::Base, QColor(42, 42, 42));
	this->darkPalette->setColor(QPalette::AlternateBase, QColor(66, 66, 66));
	this->darkPalette->setColor(QPalette::ToolTipBase, Qt::white);
	this->darkPalette->setColor(QPalette::ToolTipText, Qt::white);
	this->darkPalette->setColor(QPalette::Text, Qt::white);
	this->darkPalette->setColor(QPalette::Button, QColor(53, 53, 53));
	this->darkPalette->setColor(QPalette::ButtonText, Qt::white);
	this->darkPalette->setColor(QPalette::Highlight, QColor(42, 130, 218));
	this->darkPalette->setColor(QPalette::HighlightedText, Qt::black);
	this->darkPalette->setColor(QPalette::PlaceholderText, Qt::white);
	this->darkPalette->setColor(QPalette::Link, QColor(42, 130, 218));
	this->darkPalette->setColor(QPalette::Disabled, QPalette::Text, Qt::transparent); // Weird white shade otherwise

	// Set white palette coloring
	this->lightPalette = new QPalette();
	this->lightPalette->setColor(QPalette::Window, QColor(240, 240, 240));
	this->lightPalette->setColor(QPalette::WindowText, Qt::black);
	this->lightPalette->setColor(QPalette::Base, QColor(240, 240, 240));
	this->lightPalette->setColor(QPalette::AlternateBase, QColor(220, 220, 220));
	this->lightPalette->setColor(QPalette::ToolTipBase, Qt::black);
	this->lightPalette->setColor(QPalette::ToolTipText, Qt::black);
	this->lightPalette->setColor(QPalette::Text, Qt::black);
	this->lightPalette->setColor(QPalette::Button, QColor(220, 220, 220));
	this->lightPalette->setColor(QPalette::ButtonText, Qt::black);
	this->lightPalette->setColor(QPalette::Highlight, QColor(42, 130, 218));
	this->lightPalette->setColor(QPalette::HighlightedText, Qt::white);
}

void ThemeManager::setTheme(QApplication *app, bool is_dark_mode)
{
	app->setPalette((is_dark_mode) ? *darkPalette : *lightPalette);
}
