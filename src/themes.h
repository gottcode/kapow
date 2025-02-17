#ifndef THEMES_H
#define THEMES_H

#include <QApplication>
#include <QObject>

class ThemeManager : public QObject
{
	Q_OBJECT
public:
	explicit ThemeManager(QObject *parent = nullptr);
	void setTheme(QApplication *app, bool is_dark_mode = true);

private:
	QPalette *darkPalette;
	QPalette *lightPalette;
};

#endif // THEMES_H
