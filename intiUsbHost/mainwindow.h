/*
 * This file is part of intiLED.
 *
 * intiLED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intiLED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#pragma once

#include <QMainWindow>

#include "usb.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_vsWhite_valueChanged(int value);
    void on_vsRoyalBlue_valueChanged(int value);
    void on_vsBlue_valueChanged(int value);
    void on_vsGreen_valueChanged(int value);
    void on_vsRed_valueChanged(int value);
    void on_vsYellow_valueChanged(int value);
    void on_vsViolet_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    Usb m_usb;
};
