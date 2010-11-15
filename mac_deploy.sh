#!/bin/bash

APP='Kapow'
VERSION='1.3.3'

macdeployqt "$APP.app" -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
