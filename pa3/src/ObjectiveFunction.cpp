#include "ObjectiveFunction.h"

#include "cstdio"

#include <cassert>
#include <climits>

Wirelength::Wirelength(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement)
{
    // Initialize the gamma value
    gamma_ = max(placement_.boundryRight() - placement_.boundryLeft(), placement_.boundryTop() - placement_.boundryBottom()) * 0.0025;
}

const double &Wirelength::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the wirelength function
    value_ = 0;
    int netNum = placement_.numNets();

    for (int i = 0; i < netNum; ++i)
    {
        Net net = placement_.net(i);
        int pinNum = net.numPins();

        // set xMax, yMax
        int xMax = INT_MIN, yMax = INT_MIN, xMin = INT_MAX, yMin = INT_MAX;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            xMax = max(xMax, int(input[id].x));
            yMax = max(yMax, int(input[id].y));
            xMin = min(xMin, int(input[id].x));
            yMin = min(yMin, int(input[id].y));
        }

        // minus or add 200 for precision
        xMax -= 200;
        yMax -= 200;
        xMin += 200;
        yMin += 200;

        // count the exp value
        double expWeightedMaxX = 0, expWeightedMinX = 0, expWeightedMaxY = 0, expWeightedMinY = 0;
        double expMaxX = 0, expMinX = 0, expMaxY = 0, expMinY = 0;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            expWeightedMaxX += exp((input[id].x - xMax) / gamma_) * input[id].x;
            expWeightedMinX += exp((-input[id].x + xMin) / gamma_) * input[id].x;
            expWeightedMaxY += exp((input[id].y - yMax) / gamma_) * input[id].y;
            expWeightedMinY += exp((-input[id].y + yMin) / gamma_) * input[id].y;
            expMaxX += exp((input[id].x - xMax) / gamma_);
            expMinX += exp((-input[id].x + xMin) / gamma_);
            expMaxY += exp((input[id].y - yMax) / gamma_);
            expMinY += exp((-input[id].y + yMin) / gamma_);
        }

        // calculate the wirelength
        value_ += expWeightedMaxX / expMaxX - expWeightedMinX / expMinX + expWeightedMaxY / expMaxY - expWeightedMinY / expMinY;
    }

    // store the input
    input_ = input;

    return value_;
}

const std::vector<Point2<double>> &Wirelength::Backward()
{
    // Compute the gradient of the function
    int netNum = placement_.numNets();

    // initialize the wirelength gradient
    for (size_t i = 0; i < grad_.size(); ++i)
    {
        grad_[i].x = 0;
        grad_[i].y = 0;
    }

    // calculate the wirelength gradient
    for (int i = 0; i < netNum; ++i)
    {
        Net net = placement_.net(i);
        int pinNum = net.numPins();

        // set xMax, yMax
        int xMax = INT_MIN, yMax = INT_MIN, xMin = INT_MAX, yMin = INT_MAX;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            xMax = max(xMax, int(input_[id].x));
            yMax = max(yMax, int(input_[id].y));
            xMin = min(xMin, int(input_[id].x));
            yMin = min(yMin, int(input_[id].y));
        }

        // minus or add 200 for precision
        xMax -= 200;
        yMax -= 200;
        xMin += 200;
        yMin += 200;

        // count the exp value
        double expWeightedMaxX = 0, expWeightedMinX = 0, expWeightedMaxY = 0, expWeightedMinY = 0;
        double expMaxX = 0, expMinX = 0, expMaxY = 0, expMinY = 0;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            expWeightedMaxX += exp((input_[id].x - xMax) / gamma_) * input_[id].x;
            expWeightedMinX += exp((-input_[id].x + xMin) / gamma_) * input_[id].x;
            expWeightedMaxY += exp((input_[id].y - yMax) / gamma_) * input_[id].y;
            expWeightedMinY += exp((-input_[id].y + yMin) / gamma_) * input_[id].y;
            expMaxX += exp((input_[id].x - xMax) / gamma_);
            expMinX += exp((-input_[id].x + xMin) / gamma_);
            expMaxY += exp((input_[id].y - yMax) / gamma_);
            expMinY += exp((-input_[id].y + yMin) / gamma_);
        }

        // calculate the gradient
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            double xMaxGrad = ((1 + input_[id].x / gamma_) * exp((input_[id].x - xMax) / gamma_) * expMaxX - expWeightedMaxX / gamma_ * exp((input_[id].x - xMax) / gamma_)) / expMaxX / expMaxX;
            double xMinGrad = ((1 - input_[id].x / gamma_) * exp((-input_[id].x + xMin) / gamma_) * expMinX + expWeightedMinX / gamma_ * exp((-input_[id].x + xMin) / gamma_)) / expMinX / expMinX;
            double yMaxGrad = ((1 + input_[id].y / gamma_) * exp((input_[id].y - yMax) / gamma_) * expMaxY - expWeightedMaxY / gamma_ * exp((input_[id].y - yMax) / gamma_)) / expMaxY / expMaxY;
            double yMinGrad = ((1 - input_[id].y / gamma_) * exp((-input_[id].y + yMin) / gamma_) * expMinY + expWeightedMinY / gamma_ * exp((-input_[id].y + yMin) / gamma_)) / expMinY / expMinY;
            grad_[id].x += xMaxGrad - xMinGrad;
            grad_[id].y += yMaxGrad - yMinGrad;
        }
    }

    return grad_;
}

Density::Density(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement), mb_(0), totalModuleArea_(0)
{
    // Initialize the bin size and the number of bins
    int moduleNum = placement.numModules();
    int outLineWidth = placement.boundryRight() - placement.boundryLeft();
    int outLineHeight = placement.boundryTop() - placement.boundryBottom();
    binSize_ = min(outLineWidth, outLineHeight) / 300;
    double totalWidth = 0, totalHeight = 0;
    for (int i = 0; i < moduleNum; ++i)
    {
        totalWidth += placement.module(i).width();
        totalHeight += placement.module(i).height();
    }
    binWidth_ = binSize_;
    binHeight_ = binSize_ * totalHeight / totalWidth;
    cout << "totalWidth: " << totalWidth << " totalHeight: " << totalHeight << endl;
    cout << "binWidth: " << binWidth_ << " binHeight: " << binHeight_ << endl;

    widthBinNum_ = outLineWidth / binWidth_;
    heightBinNum_ = outLineHeight / binHeight_;

    // resize the vector to widthBinNum_ * heightBinNum_ to store the density and gradient
    binDensity_.resize(widthBinNum_);
    binDensityGrad_.resize(widthBinNum_);
    for (int i = 0; i < widthBinNum_; ++i)
    {
        binDensity_[i].resize(heightBinNum_);
        binDensityGrad_[i].resize(heightBinNum_);
        for (int j = 0; j < heightBinNum_; ++j)
        {
            binDensity_[i][j] = 0;
            binDensityGrad_[i][j] = 0;
        }
    }

    // calculate the target density of the bin and the total area of the modules
    for (int i = 0; i < moduleNum; ++i)
    {
        mb_ += placement.module(i).width() * placement.module(i).height() / outLineWidth / outLineHeight;
        totalModuleArea_ += placement.module(i).width() * placement.module(i).height();
    }
    mb_ = 0.9;
    cout << "mb: " << mb_ << endl;
}

const double &Density::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the density function

    // Initialize the value and overflow ratio
    value_ = 0;
    overflowRatio_ = 0;
    for (int i = 0; i < widthBinNum_; ++i)
    {
        for (int j = 0; j < heightBinNum_; ++j)
        {
            binDensity_[i][j] = 0;
        }
    }

    // calculate the density
    int moduleNum = placement_.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        int startBinX = (input[i].x - moduleWidth / 2 - placement_.boundryLeft()) / binWidth_ < 0 ? 0 : (input[i].x - moduleWidth / 2 - placement_.boundryLeft()) / binWidth_;
        int endBinX = (input[i].x + moduleWidth / 2 - placement_.boundryLeft()) / binWidth_ >= widthBinNum_ ? widthBinNum_ - 1 : (input[i].x + moduleWidth / 2 - placement_.boundryLeft()) / binWidth_;
        int startBinY = (input[i].y - moduleHeight / 2 - placement_.boundryBottom()) / binHeight_ < 0 ? 0 : (input[i].y - moduleHeight / 2 - placement_.boundryBottom()) / binHeight_;
        int endBinY = (input[i].y + moduleHeight / 2 - placement_.boundryBottom()) / binHeight_ >= heightBinNum_ ? heightBinNum_ - 1 : (input[i].y + moduleHeight / 2 - placement_.boundryBottom()) / binHeight_;
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                double inputX = abs((input[i].x - placement_.boundryLeft()) - (j * binWidth_ + binWidth_ * 0.5));
                double inputY = abs((input[i].y - placement_.boundryBottom()) - (k * binHeight_ + binHeight_ * 0.5));
                double ax = 4.0 / (moduleWidth + 2 * binWidth_) / (moduleWidth + 4 * binWidth_), bx = 2.0 / binWidth_ / (moduleWidth + 4 * binWidth_);
                double ay = 4.0 / (moduleHeight + 2 * binHeight_) / (moduleHeight + 4 * binHeight_), by = 2.0 / binHeight_ / (moduleHeight + 4 * binHeight_);
                double Px, Py;

                // calculate Px
                if (abs(inputX) >= 0 && abs(inputX) <= binWidth_ + moduleWidth / 2.0)
                {
                    Px = 1 - ax * inputX * inputX;
                }
                else if (abs(inputX) > binWidth_ + moduleWidth / 2.0 && abs(inputX) <= 2 * binWidth_ + moduleWidth / 2.0)
                {
                    Px = bx * (abs(inputX) - moduleWidth / 2.0 - 2 * binWidth_) * (abs(inputX) - moduleWidth / 2.0 - 2 * binWidth_);
                }
                else
                {
                    Px = 0;
                }

                // calculate Py
                if (abs(inputY) >= 0 && abs(inputY) <= binHeight_ + moduleHeight / 2.0)
                {
                    Py = 1 - ay * inputY * inputY;
                }
                else if (abs(inputY) > binHeight_ + moduleHeight / 2.0 && abs(inputY) <= 2 * binHeight_ + moduleHeight / 2.0)
                {
                    Py = by * (abs(inputY) - moduleHeight / 2.0 - 2 * binHeight_) * (abs(inputY) - moduleHeight / 2.0 - 2 * binHeight_);
                }
                else
                {
                    Py = 0;
                }

                binDensity_[j][k] += Px * Py;
            }
        }
    }

    // calculate the value and covered bin number
    int coveredBinNum = 0;
    for (int i = 0; i < widthBinNum_; ++i)
    {
        for (int j = 0; j < heightBinNum_; ++j)
        {
            value_ += (binDensity_[i][j] - mb_) * (binDensity_[i][j] - mb_);
            if (binDensity_[i][j] > 0)
            {
                coveredBinNum++;
            }
        }
    }

    // calculate the overflow ratio
    overflowRatio_ = totalModuleArea_ / coveredBinNum / binWidth_ / binHeight_ - 1;

    // store the input
    input_ = input;

    return value_;
}

const std::vector<Point2<double>> &Density::Backward()
{
    // Compute the gradient of the function

    // Initialize the density gradient
    for (size_t i = 0; i < grad_.size(); ++i)
    {
        grad_[i].x = 0;
        grad_[i].y = 0;
    }

    // calculate the density gradient
    int moduleNum = placement_.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        int startBinX = (input_[i].x - moduleWidth / 2 - placement_.boundryLeft()) / binWidth_ < 0 ? 0 : (input_[i].x - moduleWidth / 2 - placement_.boundryLeft()) / binWidth_;
        int endBinX = (input_[i].x + moduleWidth / 2 - placement_.boundryLeft()) / binWidth_ >= widthBinNum_ ? widthBinNum_ - 1 : (input_[i].x + moduleWidth / 2 - placement_.boundryLeft()) / binWidth_;
        int startBinY = (input_[i].y - moduleHeight / 2 - placement_.boundryBottom()) / binHeight_ < 0 ? 0 : (input_[i].y - moduleHeight / 2 - placement_.boundryBottom()) / binHeight_;
        int endBinY = (input_[i].y + moduleHeight / 2 - placement_.boundryBottom()) / binHeight_ >= heightBinNum_ ? heightBinNum_ - 1 : (input_[i].y + moduleHeight / 2 - placement_.boundryBottom()) / binHeight_;
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                double inputX = (input_[i].x - placement_.boundryLeft()) - (j * binWidth_ + binWidth_ * 0.5);
                double inputY = (input_[i].y - placement_.boundryBottom()) - (k * binHeight_ + binHeight_ * 0.5);
                double ax = 4.0 / (moduleWidth + 2 * binWidth_) / (moduleWidth + 4 * binWidth_), bx = 2.0 / binWidth_ / (moduleWidth + 4 * binWidth_);
                double ay = 4.0 / (moduleHeight + 2 * binHeight_) / (moduleHeight + 4 * binHeight_), by = 2.0 / binHeight_ / (moduleHeight + 4 * binHeight_);
                double Px, Py, dPx, dPy;

                // calculate Px
                if (abs(inputX) >= 0 && abs(inputX) <= binWidth_ + moduleWidth / 2.0)
                {
                    Px = 1 - ax * inputX * inputX;
                }
                else if (abs(inputX) > binWidth_ + moduleWidth / 2.0 && abs(inputX) <= 2 * binWidth_ + moduleWidth / 2.0)
                {
                    Px = bx * (abs(inputX) - moduleWidth / 2.0 - 2 * binWidth_) * (abs(inputX) - moduleWidth / 2.0 - 2 * binWidth_);
                }
                else
                {
                    Px = 0;
                }

                // calculate Py
                if (abs(inputY) >= 0 && abs(inputY) <= binHeight_ + moduleHeight / 2.0)
                {
                    Py = 1 - ay * inputY * inputY;
                }
                else if (abs(inputY) > binHeight_ + moduleHeight / 2.0 && abs(inputY) <= 2 * binHeight_ + moduleHeight / 2.0)
                {
                    Py = by * (abs(inputY) - moduleHeight / 2.0 - 2 * binHeight_) * (abs(inputY) - moduleHeight / 2.0 - 2 * binHeight_);
                }
                else
                {
                    Py = 0;
                }

                // calculate dPx
                if (abs(inputX) >= 0 && abs(inputX) <= binWidth_ + moduleWidth / 2.0)
                {
                    dPx = -2 * ax * inputX;
                }
                else if (abs(inputX) > binWidth_ + moduleWidth / 2.0 && abs(inputX) <= 2 * binWidth_ + moduleWidth / 2)
                {
                    if (inputX > 0)
                    {
                        dPx = 2 * bx * (inputX - moduleWidth / 2.0 - 2 * binWidth_);
                    }
                    else
                    {
                        dPx = 2 * bx * (inputX + moduleWidth / 2.0 + 2 * binWidth_);
                    }
                }
                else
                {
                    dPx = 0;
                }

                // calculate dPy
                if (abs(inputY) >= 0 && abs(inputY) <= binHeight_ + moduleHeight / 2.0)
                {
                    dPy = -2 * ay * inputY;
                }
                else if (abs(inputY) > binHeight_ + moduleHeight / 2.0 && abs(inputY) <= 2 * binHeight_ + moduleHeight / 2.0)
                {
                    if (inputY > 0)
                    {
                        dPy = 2 * by * (inputY - moduleHeight / 2.0 - 2 * binHeight_);
                    }
                    else
                    {
                        dPy = 2 * by * (inputY + moduleHeight / 2.0 + 2 * binHeight_);
                    }
                }
                else
                {
                    dPy = 0;
                }

                grad_[i].x += 2 * (binDensity_[j][k] - mb_) * dPx * Py;
                grad_[i].y += 2 * (binDensity_[j][k] - mb_) * Px * dPy;
            }
        }
    }

    return grad_;
}

const double &ObjectiveFunction::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the objective function
    value_ = wirelength_(input) + lambda_ * density_(input);

    // store the input
    input_ = input;

    return value_;
}

const std::vector<Point2<double>> &ObjectiveFunction::Backward()
{
    // Compute the gradient of the function
    vector<Point2<double>> wirelengthGrad, densityGrad;
    wirelengthGrad = wirelength_.Backward();
    densityGrad = density_.Backward();
    int moduleNum = placement_.numModules();

    // update lambda, before 10 iterations, lambda is 0, after 200 iterations and not spread enough, lambda *= 1.1
    if (iterNum_ == 0)
    {
        double wirelengthGradNorm = 0, densityGradNorm = 0;
        for (int i = 0; i < moduleNum; ++i)
        {
            wirelengthGradNorm += Norm2(wirelengthGrad[i]);
            densityGradNorm += Norm2(densityGrad[i]);
        }
        lambda_ = wirelengthGradNorm / densityGradNorm * 0.8;
    }

    // calculate the gradient
    for (int i = 0; i < moduleNum; ++i)
    {
        grad_[i] = wirelengthGrad[i] + lambda_ * densityGrad[i];
    }

    iterNum_++;

    return grad_;
}