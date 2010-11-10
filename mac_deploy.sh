#!/bin/bash

APP='Kapow'
VERSION='1.3.2'

macdeployqt "$APP.app" -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
