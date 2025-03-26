#include "oiioimagedata.h"

#include <QDebug>
#include <QMutex>
#include <QRect>

#include <opencv2/opencv.hpp>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebufalgo.h>

#include "texiftoolmetadata.h"

OIIOImageData::OIIOImageData() : ImageData<OIIO::ImageBuf>(), m_image(OIIO::ImageBuf())
{

}

OIIOImageData::OIIOImageData(const QString &filePath, const OIIO::ImageSpec &config, OIIO::ImageCache *cache)
    : m_image(filePath.toStdString(), 0, 0, cache, &config)
{
    m_roi = QRect(0, 0, m_image.oriented_full_width(), m_image.oriented_full_height());
}

OIIOImageData::OIIOImageData(const OIIO::ImageBuf &mat)
    : ImageData<OIIO::ImageBuf>(), m_image(mat)
{
    m_roi = QRect(0, 0, m_image.oriented_full_width(), m_image.oriented_full_height());
}

OIIOImageData::~OIIOImageData() {
}

void OIIOImageData::cloneFrom(const OIIO::ImageBuf &src) {
    m_image = src;
    m_roi = QRect(0, 0, m_image.oriented_full_width(), m_image.oriented_full_height());
}

void OIIOImageData::copyFrom(const OIIO::ImageBuf &src) {
    m_image.copy(src);
    m_roi = QRect(0, 0, src.oriented_full_width(), src.oriented_full_height());
}

cv::Mat OIIOImageData::get(TRect roi, int chanStart, int chanEnd) const {
    if (roi.isEmpty()) {
        roi = TRect(0, 0, width(), height());
    }

    int chbegin = chanStart;
    int chend = chanEnd;
    int chtotal = chend - chbegin;

    if (chend < 0 || chend > m_image.nchannels() || chtotal > m_image.nchannels()) {
        chend = m_image.nchannels();
    }

    OIIO::ROI oiioRoi(roi.x(), roi.x() + roi.width(),   // X
                      roi.y(), roi.y() + roi.height(),  // Y
                      0, 1,                             // Z
                      chbegin, chend);                  // C

    OIIO::ImageSpec spec(oiioRoi, m_image.pixeltype());
    OIIO::ImageBuf output(spec);

    m_image.get_pixels(oiioRoi, m_image.pixeltype(), output.localpixels());

    cv::Mat outputMat;
    OIIO::ImageBufAlgo::to_OpenCV(outputMat, output);
    return outputMat;
}

cv::Mat OIIOImageData::cropAndRotate(QRectF cropRect, const double rotation, bool subtractBounds) const {
    if (cropRect.isEmpty()) {
        return getImage();
    }

    // auto rotatedRect = CvUtils::getRotatedRect(cropRect, rotation);
    return {}; //CvUtils::applyRotation(getImage(), rotatedRect, subtractBounds);
}

bool OIIOImageData::hasAlpha() const {
    return m_image.nchannels() == 4;
}

bool OIIOImageData::imwrite(const QString &file, bool is16Bit) const {
    if (empty()) {
        qWarning() << "Ignoring attempt to write an empty mat";
        return false;
    }

    return imwrite(file, m_image, is16Bit);
}

bool OIIOImageData::imwrite(const QString &file, const OIIO::ImageBuf &image, bool is16Bit) {
    return image.write(file.toStdString(), is16Bit ? OIIO::TypeDesc::UINT16 : OIIO::TypeDesc::UINT8);;
}

bool OIIOImageData::empty() const {
    return !m_image.initialized() || width() == 0 || height() == 0;
}

int OIIOImageData::width() const {
    return m_image.oriented_full_width();
}

int OIIOImageData::height() const {
    return m_image.oriented_full_height();
}

int OIIOImageData::channels() const {
    return m_image.nchannels();
}

QSize OIIOImageData::size() const {
    return QSize(width(), height());
}

void OIIOImageData::rotate(int orientation) {
    if (orientation <= Orientation::Invalid || orientation > Orientation::MaxOrientation) {
        return;
    }

    rotate(orientation, m_image);

    // NOTE: This does not handle X/Y offset when orientation changes
    if (orientation >= Orientation::MirrorHorizontalAndRotate270CW) {
        m_roi = m_roi.transposed();
    }
}

cv::Mat OIIOImageData::resized(QSize size, TRect roi) const {
    // TODO: ImageBufAlgo::resize
    return {};
}

void OIIOImageData::rotate(int orientation, OIIO::ImageBuf &mat) {
    // Handle orientation
    // TODO: Should this type of pre-processing be put into another section?
    switch(orientation) {
    case Orientation::Horizontal:
        break;
    case Orientation::MirrorHorizontal:
        mat = OIIO::ImageBufAlgo::flop(mat);
        break;
    case Orientation::Rotate180:
        mat = OIIO::ImageBufAlgo::rotate180(mat);
        break;
    case Orientation::MirrorVertical:
        mat = OIIO::ImageBufAlgo::flip(mat);
        break;
    case Orientation::MirrorHorizontalAndRotate270CW:
        mat = OIIO::ImageBufAlgo::transpose(mat);
        break;
    case Orientation::Rotate90CW:
        mat = OIIO::ImageBufAlgo::rotate90(mat);
        break;
    case Orientation::MirrorHorizontalAndRotate90CW:
        mat = OIIO::ImageBufAlgo::flop(mat);
        mat = OIIO::ImageBufAlgo::rotate90(mat);
        break;
    case Orientation::Rotate270CW:
        mat = OIIO::ImageBufAlgo::rotate270(mat);
        break;
    default:
        qWarning() << "Orientation: Invalid value" << orientation;
        return;
    }
}

size_t OIIOImageData::memoryUsage() const {
    return 0;
}

bool OIIOImageData::isRoiValid(cv::Rect roi, cv::Size size) const {
    cv::Rect bounds(0, 0, size.width, size.height);
    cv::Rect intersection = bounds & roi;

    // Intersection will return the original ROI if it's fully inbounds
    if (intersection != roi) {
        qWarning() << "ROI out of bounds";
        return false;
    }

    return true;
}
