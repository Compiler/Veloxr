#pragma once

#include <QColorSpace>
#include <QObject>
#include <QRect>

#include <OpenImageIO/imagebuf.h>

#include "imagedata.h"
#include "imagedata_export.h"

class IMAGEDATA_EXPORT OIIOImageData : public ImageData<OIIO::ImageBuf> {
public:
    OIIOImageData();
    OIIOImageData(const QString &filePath, const OIIO::ImageSpec &config, OIIO::ImageCache *cache = nullptr);
    OIIOImageData(const OIIO::ImageBuf &mat);
    OIIOImageData(const OIIOImageData &other) = delete;
    ~OIIOImageData();

    /**
     * @brief cloneFrom Replaces the data in the underlying mat with the given src
     * @param src The data to replace the underlying data with
     */
    void cloneFrom(const OIIO::ImageBuf &src) override;

    /**
     * @brief copyFrom Replaces the data in the underlying mat with the given src
     * @param src The data to replace the underlying data with
     */
    void copyFrom(const OIIO::ImageBuf &src) override;

    cv::Mat get(TRect roi, int chanStart = 0, int chanEnd = -1) const override;

    /**
     * @brief hasAlpha Whether or not this image has an alpha channel
     * @return True if it does, false otherwise
     */
    bool hasAlpha() const override;

    int width() const override;
    int height() const override;

    /**
     * @brief imwrite Write the current imagedata to a file
     * @param file The file path to write to, including the extension (.png, .jpg, etc)
     * @return True on success, false on failure. Always returns false outside of DEV mode.
     */
    bool imwrite(const QString &file, bool is16Bit = false) const override;

    static bool imwrite(const QString &file, const OIIO::ImageBuf &image, bool is16Bit = false);

    /**
     * @brief empty Whether or not the underlying mat is empty
     * @return True if underlying RGB mat is empty, false otherwise. Does not take alpha into account.
     */
    bool empty() const override;

    /**
     * @brief channels Get the total amount of channels this image has
     * @return Total of image mat + alpha mat channels
     */
    int channels() const override;

    /**
     * @brief size Get the size of the underlying mat
     * @return Dimensions of the underlying mat potentially taking crop into account
     */
    QSize size() const override;

    // Potential internal modification methods

    /**
     * @brief rotate Rotate the internal mat
     * @param orientation
     */
    void rotate(int orientation) override;

    cv::Mat resized(QSize size, TRect roi) const override;

    /**
     * @brief rotate Rotate the internal mat
     * @param orientation
     * @param mat The mat to rotate
     */
    static void rotate(int orientation, OIIO::ImageBuf &mat);

    size_t memoryUsage() const override;

    cv::Mat cropAndRotate(QRectF cropRect, const double rotation, bool subtractBounds) const;

private:
    OIIO::ImageBuf m_image;

    bool isRoiValid(cv::Rect roi, cv::Size size) const;
};
