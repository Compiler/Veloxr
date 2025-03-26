#pragma once

#include "imagedata_export.h"
#include "trect.h"
#include "colortransform.h"

#include <opencv2/opencv.hpp>

template<typename ImageType>
class IMAGEDATA_EXPORT ImageData {
public:
    ImageData() : m_roi(0, 0, 0, 0), m_cropRoi(0, 0, 0, 0), m_colorTransform(nullptr) {}
    ImageData(const ImageData &other) = delete;
    virtual ~ImageData() {};

    /**
     * @brief cloneFrom Replaces the data in the underlying mat with the given src
     * @param src The data to replace the underlying data with
     */
    virtual void cloneFrom(const ImageType &src) = 0;

    /**
     * @brief copyFrom Replaces the data in the underlying mat with the given src
     * @param src The data to replace the underlying data with
     */
    virtual void copyFrom(const ImageType &src) = 0;

    virtual cv::Mat getImage(int chanStart = 0, int chanEnd = -1) const {
        return get(roi(), chanStart, chanEnd);
    }

    virtual cv::Mat get(TRect roi, int chanStart = 0, int chanEnd = -1) const = 0;

    /**
     * @brief roi Get the internal un-applied cropROI.
     * @return Un-applied crop ROI
     */
    virtual TRect roi() const {
        return m_roi;
    }

    virtual void setRoi(TRect roi) {
        m_roi = roi;
    }

    /**
     * @brief cropRoi Get the internal cropROI.
     * @return Crop ROI
     */
    virtual TRect cropRoi() const {
        return m_cropRoi;
    };

    /**
     * @brief hasAlpha Whether or not this image has an alpha channel
     * @return True if it does, false otherwise
     */
    virtual bool hasAlpha() const = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;

    /**
     * @brief empty Whether or not the underlying mat is empty
     * @return True if underlying RGB mat is empty, false otherwise. Does not take alpha into account.
     */
    virtual bool empty() const = 0;

    /**
     * @brief channels Get the total amount of channels this image has
     * @return Total of image mat + alpha mat channels
     */
    virtual int channels() const = 0;

    /**
     * @brief size Get the size of the underlying mat
     * @return Dimensions of the underlying mat potentially taking crop into account
     */
    virtual QSize size() const = 0;

    /**
     * @brief rotate Rotate the internal mat
     * @param orientation
     */
    virtual void rotate(int orientation) = 0;

    virtual cv::Mat resized(QSize size, TRect roi) const = 0;

    virtual std::shared_ptr<ColorTransform> colorTransform() const {
        return m_colorTransform;
    }

    virtual void setColorTransform(std::shared_ptr<ColorTransform> transform) {
        m_colorTransform = transform;
    }

    virtual size_t memoryUsage() const = 0;

    /**
     * @brief imwrite Write the current imagedata to a file
     * @param file The file path to write to, including the extension (.png, .jpg, etc)
     * @return True on success, false on failure. Always returns false outside of DEV mode.
     */
    virtual bool imwrite(const QString &file, bool is16Bit = false) const = 0;

protected:
    /// The current ROI to crop to when returning mat information
    TRect m_roi;
    /// The internal crop ROI
    TRect m_cropRoi;
    /// The color transform for the input image
    /// This should be a transform from the image reader where:
    ///     From -> To
    /// Input CS -> ProPhoto
    /// This allows us to inverse the ProPhoto transform later.
    std::shared_ptr<ColorTransform> m_colorTransform;
};
