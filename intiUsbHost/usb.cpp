#include <qdebug.h>

#include "usb.h"
#include <message.h>

Usb::Usb(short vid, short pid, QObject *parent)
    : QObject(parent),
      m_vid(vid),
      m_pid(pid),
      m_thread(0),
      m_endpoint(1)
{
// enumerate all USB devices to try and find our specified device
    int error_code = libusb_init(&m_context);

    if (error_code < 0)
    {
        throw error_code;
    }

    if (!initDevice())
        throw "No device found";
}

Usb::~Usb()
{
    close();

    //Exit from libusb library
    libusb_exit(m_context);
}

bool Usb::initDevice()
{
    bool found = false;
    //USB devices
    libusb_device ** devs;

    // detect all devices on the USB bus
    // and select our vid pid pair
    if ((m_error = libusb_get_device_list(m_context, &devs)) > 0)
    {
        int i = 0;

        while (!found && (m_device = devs[i++]) != NULL)
        {
            struct libusb_device_descriptor desc;

            m_error = libusb_get_device_descriptor((libusb_device*)m_device, &desc);

            if (!m_error)
            {
                // check if our VID and PID match
                found = (m_vid == desc.idVendor && m_pid == desc.idProduct);

                if (found)
                {
                    m_error = libusb_open((libusb_device*)m_device,
                                          (libusb_device_handle**)&m_devicehandle);

                    if (m_error)
                        found = false;

                    break;
                }
            }
        }
    }

    return found;
}
void Usb::close()
{
    libusb_release_interface((libusb_device_handle*)m_devicehandle, 0);
    libusb_close((libusb_device_handle*)m_devicehandle);
}
bool Usb::start()
{
    bool ret = true;

    // generate our RX thread
    if (!m_thread)
    {
        pthread_mutex_lock(&m_mutex);
        m_stop = false;
        pthread_mutex_unlock(&m_mutex);

        int s = pthread_create(&m_thread, NULL, &Usb::rx_thread, this);

        if (s != 0)
            ret = false;
//        else
//            ret = submit_buffers();
    }
    else
        ret = false;

    if (ret)
    {
        m_transfers[1] = libusb_alloc_transfer(0);

        libusb_fill_interrupt_transfer(m_transfers[1], (libusb_device_handle*)m_devicehandle,
                2, (uchar*)m_buffer[1], 24, &call_fn, this, 0);

        // resubmitt our buffer
        m_error = libusb_submit_transfer(m_transfers[1]);

        return !m_error;
    }

    return ret;
}
bool Usb::stop()
{
    bool ret = true;

    // stop our RX thread
    if (!m_thread)
    {
        pthread_mutex_lock(&m_mutex);
        m_stop = true;
        pthread_mutex_unlock(&m_mutex);

        int s = pthread_join(m_thread, NULL);

        if (s != 0)
            ret = false;
    }

    return ret;
}
bool Usb::send(uchar * data, uint len)
{
    m_transfers[0] = libusb_alloc_transfer(0);

    libusb_fill_interrupt_transfer(m_transfers[0], (libusb_device_handle*)m_devicehandle,
                              m_endpoint, data, len, &call_fn, this, 0);

    // resubmitt our buffer
    m_error = libusb_submit_transfer(m_transfers[0]);

    return !m_error;
}
void Usb::call_fn(libusb_transfer * transfer)
{
    Usb * pthis = (Usb*)transfer->user_data;

    switch (transfer->status)
    {
    case LIBUSB_TRANSFER_COMPLETED:
        break;

    case LIBUSB_TRANSFER_TIMED_OUT:
        qDebug() << "Timeout";
        break;

    case LIBUSB_TRANSFER_ERROR:
        qDebug() << "Error with submitted buffer";
        break;

    case LIBUSB_TRANSFER_CANCELLED:
        qDebug() << "Transfer Cancelled";
        break;

    case LIBUSB_TRANSFER_STALL:
        qDebug() << "Transfer STALL";
        break;

    case LIBUSB_TRANSFER_NO_DEVICE:
        qDebug() << "Error no device";
        break;

    case LIBUSB_TRANSFER_OVERFLOW:
        qDebug() << "Transfer Overflow";
        break;

    default:
        qDebug() << "Error with submitted buffer" << transfer->status;
        break;
    }

    // resubmitt our buffer
    libusb_submit_transfer(transfer);
}
void * Usb::rx_thread(void * param)
{
    Usb * pthis = (Usb*)param;

    bool exit = false;

    while (!exit)
    {
        if (pthis->m_context)
            libusb_handle_events(pthis->m_context);

        pthread_mutex_lock  (&pthis->m_mutex);
        exit = pthis->m_stop;
        pthread_mutex_unlock(&pthis->m_mutex);

    }

    return pthis;
}
