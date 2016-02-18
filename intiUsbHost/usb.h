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

#include <QObject>
#include <libusb.h>

class Usb : public QObject
{
    Q_OBJECT
public:
    explicit Usb(short vid, short pid, QObject *parent = 0);
    ~Usb();

    bool start();
    bool stop();

    bool send(char *data, uint len);

signals:

public slots:

private:
    static void * rx_thread(void * param);
    bool initDevice();
    void close();

    const short m_vid;
    const short m_pid;

    // call back function
    static void call_fn(libusb_transfer * transfer);
    bool submit_buffers();

    // USB device and device handles
    void * m_device;
    void * m_devicehandle;
    libusb_context * m_context;

    unsigned char m_endpoint;

    int m_error;

    //
    // Thread control
    //
    pthread_t         m_thread;
    pthread_mutex_t   m_mutex;
    bool              m_stop;

    static const int NBUFFERS = 5;
    libusb_transfer * m_transfers[NBUFFERS];

    char m_buffer[5][24];
};
