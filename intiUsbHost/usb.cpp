#include <qdebug.h>

#include "usb.h"
#include <message.h>

// Values for bmRequestType in the Setup transaction's Data packet.

static const int CONTROL_REQUEST_TYPE_IN = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
static const int CONTROL_REQUEST_TYPE_OUT = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;

// From the HID spec:

static const int HID_GET_REPORT = 0x01;
static const int HID_SET_REPORT = 0x09;
static const int HID_REPORT_TYPE_INPUT = 0x01;
static const int HID_REPORT_TYPE_OUTPUT = 0x02;
static const int HID_REPORT_TYPE_FEATURE = 0x03;

// With firmware support, transfers can be > the endpoint's max packet size.

static const int MAX_CONTROL_IN_TRANSFER_SIZE = 2;
static const int MAX_CONTROL_OUT_TRANSFER_SIZE = 2;

static const int INTERFACE_NUMBER = 0;
static const int TIMEOUT_MS = 5000;


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

    for (int i = 0; i< NBUFFERS; i++)
        m_transfers[i] = libusb_alloc_transfer(0);

    pthread_mutex_init(&m_mutex, NULL);
}

Usb::~Usb()
{
    close();

    for (int i = 0; i< NBUFFERS; i++)
         libusb_free_transfer(m_transfers[i]);

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
bool Usb::send(char * data, uint len)
{
    data[0] = 0x81;
    m_error = libusb_control_transfer(
            (libusb_device_handle*)m_devicehandle, LIBUSB_REQUEST_SET_COvNFIGURATION,
                /*0x21*/ LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                0, 0, (uchar*)data, len, 0);

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
    libusb_free_transfer(transfer);
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
