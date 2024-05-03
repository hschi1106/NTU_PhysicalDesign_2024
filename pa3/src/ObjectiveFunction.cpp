#include "ObjectiveFunction.h"

#include "cstdio"

#include <cassert>
#include <climits>

ExampleFunction::ExampleFunction(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement)
{
    printf("Fetch the information you need from placement database.\n");
    printf("For example:\n");
    printf("    Placement boundary: (%.f,%.f)-(%.f,%.f)\n", placement_.boundryLeft(), placement_.boundryBottom(),
           placement_.boundryRight(), placement_.boundryTop());
}

const double &ExampleFunction::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the function
    value_ = 3. * input[0].x * input[0].x + 2. * input[0].x * input[0].y +
             2. * input[0].y * input[0].y + 7.;
    input_ = input;
    return value_;
}

const std::vector<Point2<double>> &ExampleFunction::Backward()
{
    // Compute the gradient of the function
    grad_[0].x = 6. * input_[0].x + 2. * input_[0].y;
    grad_[0].y = 2. * input_[0].x + 4. * input_[0].y;
    return grad_;
}

Wirelength::Wirelength(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement)
{
    gamma_ = max(placement_.boundryRight() - placement_.boundryLeft(), placement_.boundryTop() - placement_.boundryBottom()) * 0.005;
}

const double &Wirelength::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the wirelength function
    value_ = 0; // Initialize the value of the wirelength function
    int netNum = placement_.numNets();

    for (int i = 0; i < netNum; ++i)
    {
        Net net = placement_.net(i);
        int pinNum = net.numPins();

        // set xMax, yMax
        int xMax = INT_MIN, yMax = INT_MIN;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            xMax = max(xMax, int(input[id].x));
            yMax = max(yMax, int(input[id].y));
        }

        // count the exp value
        double expWeightedMaxX = 0, expWeightedMinX = 0, expWeightedMaxY = 0, expWeightedMinY = 0;
        double expMaxX = 0, expMinX = 0, expMaxY = 0, expMinY = 0;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            expWeightedMaxX += exp((input[id].x - xMax) / gamma_) * input[id].x;
            expWeightedMinX += exp(-(input[id].x - xMax) / gamma_) * input[id].x;
            expWeightedMaxY += exp((input[id].y - yMax) / gamma_) * input[id].y;
            expWeightedMinY += exp(-(input[id].y - yMax) / gamma_) * input[id].y;
            expMaxX += exp((input[id].x - xMax) / gamma_);
            expMinX += exp(-(input[id].x - xMax) / gamma_);
            expMaxY += exp((input[id].y - yMax) / gamma_);
            expMinY += exp(-(input[id].y - yMax) / gamma_);
        }
        value_ += expWeightedMaxX / expMaxX - expWeightedMinX / expMinX + expWeightedMaxY / expMaxY - expWeightedMinY / expMinY;
    }

    input_ = input;

    return value_;
}

const std::vector<Point2<double>> &Wirelength::Backward()
{
    // Compute the gradient of the function
    int netNum = placement_.numNets();

    // initialize the gradient
    for (size_t i = 0; i < grad_.size(); ++i)
    {
        grad_[i].x = 0;
        grad_[i].y = 0;
    }

    // calculate the gradient
    for (int i = 0; i < netNum; ++i)
    {
        Net net = placement_.net(i);
        int pinNum = net.numPins();

        // set xMax, yMax
        int xMax = INT_MIN, yMax = INT_MIN;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            xMax = max(xMax, int(input_[id].x));
            yMax = max(yMax, int(input_[id].y));
        }

        // count the exp value
        double expWeightedMaxX = 0, expWeightedMinX = 0, expWeightedMaxY = 0, expWeightedMinY = 0;
        double expMaxX = 0, expMinX = 0, expMaxY = 0, expMinY = 0;
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            expWeightedMaxX += exp((input_[id].x - xMax) / gamma_) * input_[id].x;
            expWeightedMinX += exp(-(input_[id].x - xMax) / gamma_) * input_[id].x;
            expWeightedMaxY += exp((input_[id].y - yMax) / gamma_) * input_[id].y;
            expWeightedMinY += exp(-(input_[id].y - yMax) / gamma_) * input_[id].y;
            expMaxX += exp((input_[id].x - xMax) / gamma_);
            expMinX += exp(-(input_[id].x - xMax) / gamma_);
            expMaxY += exp((input_[id].y - yMax) / gamma_);
            expMinY += exp(-(input_[id].y - yMax) / gamma_);
        }

        // calculate the gradient
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            double xMaxGrad = ((1 + input_[id].x / gamma_) * exp((input_[id].x - xMax) / gamma_) * expMaxX - expWeightedMaxX / gamma_ * exp((input_[id].x - xMax) / gamma_)) / expMaxX / expMaxX;
            double xMinGrad = ((1 - input_[id].x / gamma_) * exp(-(input_[id].x - xMax) / gamma_) * expMinX + expWeightedMinX / gamma_ * exp(-(input_[id].x - xMax) / gamma_)) / expMinX / expMinX;
            double yMaxGrad = ((1 + input_[id].y / gamma_) * exp((input_[id].y - yMax) / gamma_) * expMaxY - expWeightedMaxY / gamma_ * exp((input_[id].y - yMax) / gamma_)) / expMaxY / expMaxY;
            double yMinGrad = ((1 - input_[id].y / gamma_) * exp(-(input_[id].y - yMax) / gamma_) * expMinY + expWeightedMinY / gamma_ * exp(-(input_[id].y - yMax) / gamma_)) / expMinY / expMinY;
            grad_[id].x += xMaxGrad - xMinGrad;
            grad_[id].y += yMaxGrad - yMinGrad;
        }
    }

    return grad_;
}

Density::Density(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement), binSize_(100)
{
    // resize the vector to widthBinNum_ * heightBinNum_ to store the density and gradient
    int outLineWidth = placement.boundryRight() - placement.boundryLeft();
    int outLineHeight = placement.boundryTop() - placement.boundryBottom();
    widthBinNum_ = outLineWidth / binSize_;
    heightBinNum_ = outLineHeight / binSize_;
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

    // calculate the target density of the bin
    int moduleNum = placement.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        mb_ += placement.module(i).width() * placement.module(i).height() / (outLineWidth * outLineHeight);
    }
    mb_ = 2;
}

const double &Density::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the density function

    // Initialize the value of the density function
    value_ = 0;
    overflowRatio_ = 0;
    for (int i = 0; i < widthBinNum_; ++i)
    {
        for (int j = 0; j < heightBinNum_; ++j)
        {
            binDensity_[i][j] = 0;
        }
    }
    int moduleNum = placement_.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        int startBinX = (input[i].x - placement_.boundryLeft()) / binSize_ < 0 ? 0 : (input[i].x - placement_.boundryLeft()) / binSize_;
        int endBinX = (input[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binSize_ >= widthBinNum_ ? widthBinNum_ - 1 : (input[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binSize_;
        int startBinY = (input[i].y - placement_.boundryBottom()) / binSize_ < 0 ? 0 : (input[i].y - placement_.boundryBottom()) / binSize_;
        int endBinY = (input[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binSize_ >= heightBinNum_ ? heightBinNum_ - 1 : (input[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binSize_;
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                // double inputX = input[i].x - placement_.boundryLeft() + moduleWidth - j * binSize_;
                // double inputY = input[i].y - placement_.boundryBottom() + moduleHeight - k * binSize_;
                double inputX = abs((input[i].x - placement_.boundryLeft() + moduleWidth / 2) - (j * binSize_ + binSize_ / 2));
                double inputY = abs((input[i].y - placement_.boundryBottom() + moduleHeight / 2) - (k * binSize_ + binSize_ / 2));
                double ax = 4 / (moduleWidth + 2 * binSize_) / (moduleWidth + 4 * binSize_), bx = 2 / binSize_ / (moduleWidth + 4 * binSize_);
                double ay = 4 / (moduleHeight + 2 * binSize_) / (moduleHeight + 4 * binSize_), by = 2 / binSize_ / (moduleHeight + 4 * binSize_);
                double Px, Py;

                // calculate Px
                if (abs(inputX) >= 0 && abs(inputX) <= binSize_ + moduleWidth / 2)
                {
                    Px = 1 - ax * inputX * inputX;
                }
                else if (abs(inputX) > binSize_ + moduleWidth / 2 && abs(inputX) <= 2 * binSize_ + moduleWidth / 2)
                {
                    Px = bx * (abs(inputX) - moduleWidth / 2 - 2 * binSize_) * (abs(inputX) - moduleWidth / 2 - 2 * binSize_);
                }
                else
                {
                    Px = 0;
                }

                // calculate Py
                if (abs(inputY) >= 0 && abs(inputY) <= binSize_ + moduleHeight / 2)
                {
                    Py = 1 - ay * inputY * inputY;
                }
                else if (abs(inputY) > binSize_ + moduleHeight / 2 && abs(inputY) <= 2 * binSize_ + moduleHeight / 2)
                {
                    Py = by * (abs(inputY) - moduleHeight / 2 - 2 * binSize_) * (abs(inputY) - moduleHeight / 2 - 2 * binSize_);
                }
                else
                {
                    Py = 0;
                }

                binDensity_[j][k] += Px * Py;
            }
        }
    }

    for (int i = 0; i < widthBinNum_; ++i)
    {
        for (int j = 0; j < heightBinNum_; ++j)
        {
            value_ += (binDensity_[i][j] - mb_) * (binDensity_[i][j] - mb_);
            overflowRatio_ += max(0.0, binDensity_[i][j] - mb_);
        }
    }

    overflowRatio_ /= widthBinNum_ * heightBinNum_;

    input_ = input;
    return value_;
}

const std::vector<Point2<double>> &Density::Backward()
{
    // Compute the gradient of the function

    // Initialize the gradient
    for (size_t i = 0; i < grad_.size(); ++i)
    {
        grad_[i].x = 0;
        grad_[i].y = 0;
    }

    // calculate the gradient
    int moduleNum = placement_.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        int startBinX = (input_[i].x - placement_.boundryLeft()) / binSize_ < 0 ? 0 : (input_[i].x - placement_.boundryLeft()) / binSize_;
        int endBinX = (input_[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binSize_ >= widthBinNum_ ? widthBinNum_ - 1 : (input_[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binSize_;
        int startBinY = (input_[i].y - placement_.boundryBottom()) / binSize_ < 0 ? 0 : (input_[i].y - placement_.boundryBottom()) / binSize_;
        int endBinY = (input_[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binSize_ >= heightBinNum_ ? heightBinNum_ - 1 : (input_[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binSize_;
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                // double inputX = input_[i].x - placement_.boundryLeft() + moduleWidth - j * binSize_;
                // double inputY = input_[i].y - placement_.boundryBottom() + moduleHeight - k * binSize_;
                double inputX = (input_[i].x - placement_.boundryLeft() + moduleWidth / 2) - (j * binSize_ + binSize_ / 2);
                double inputY = (input_[i].y - placement_.boundryBottom() + moduleHeight / 2) - (k * binSize_ + binSize_ / 2);
                double ax = 4 / (moduleWidth + 2 * binSize_) / (moduleWidth + 4 * binSize_), bx = 2 / binSize_ / (moduleWidth + 4 * binSize_);
                double ay = 4 / (moduleHeight + 2 * binSize_) / (moduleHeight + 4 * binSize_), by = 2 / binSize_ / (moduleHeight + 4 * binSize_);
                double Px, Py, dPx, dPy;

                // calculate Px
                if (abs(inputX) >= 0 && abs(inputX) <= binSize_ + moduleWidth / 2)
                {
                    Px = 1 - ax * inputX * inputX;
                }
                else if (abs(inputX) > binSize_ + moduleWidth / 2 && abs(inputX) <= 2 * binSize_ + moduleWidth / 2)
                {
                    Px = bx * (abs(inputX) - moduleWidth / 2 - 2 * binSize_) * (abs(inputX) - moduleWidth / 2 - 2 * binSize_);
                }
                else
                {
                    Px = 0;
                }

                // calculate Py
                if (abs(inputY) >= 0 && abs(inputY) <= binSize_ + moduleHeight / 2)
                {
                    Py = 1 - ay * inputY * inputY;
                }
                else if (abs(inputY) > binSize_ + moduleHeight / 2 && abs(inputY) <= 2 * binSize_ + moduleHeight / 2)
                {
                    Py = by * (abs(inputY) - moduleHeight / 2 - 2 * binSize_) * (abs(inputY) - moduleHeight / 2 - 2 * binSize_);
                }
                else
                {
                    Py = 0;
                }

                // calculate dPx
                if (abs(inputX) >= 0 && abs(inputX) <= binSize_ + moduleWidth / 2)
                {
                    dPx = -2 * ax * inputX;
                }
                else if (abs(inputX) > binSize_ + moduleWidth / 2 && abs(inputX) <= 2 * binSize_ + moduleWidth / 2)
                {
                    if (inputX > 0)
                    {
                        dPx = 2 * bx * (inputX - moduleWidth / 2 - 2 * binSize_);
                    }
                    else
                    {
                        dPx = -2 * bx * (inputX - moduleWidth / 2 - 2 * binSize_);
                    }
                }
                else
                {
                    dPx = 0;
                }

                // calculate dPy
                if (abs(inputY) >= 0 && abs(inputY) <= binSize_ + moduleHeight / 2)
                {
                    dPy = -2 * ay * inputY;
                }
                else if (abs(inputY) > binSize_ + moduleHeight / 2 && abs(inputY) <= 2 * binSize_ + moduleHeight / 2)
                {
                    if (inputY > 0)
                    {
                        dPy = 2 * by * (inputY - moduleHeight / 2 - 2 * binSize_);
                    }
                    else
                    {
                        dPy = -2 * by * (inputY - moduleHeight / 2 - 2 * binSize_);
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
    double wirelengthCost, densityCost;
    wirelengthCost = wirelength_(input);
    densityCost = density_(input);
    cout << "wirelengthCost: " << wirelengthCost << " densityCost: " << densityCost << endl;
    value_ = wirelengthCost + lambda_ * densityCost;
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

    if (iterNum_ == 0)
    {
        double wirelengthGradNorm = 0, densityGradNorm = 0;
        for (int i = 0; i < moduleNum; ++i)
        {
            wirelengthGradNorm += sqrt(wirelengthGrad[i].x * wirelengthGrad[i].x + wirelengthGrad[i].y * wirelengthGrad[i].y);
            densityGradNorm += sqrt(densityGrad[i].x * densityGrad[i].x + densityGrad[i].y * densityGrad[i].y);
        }
        lambda_ = wirelengthGradNorm / densityGradNorm * 0.9;
    }
    else
    {
        lambda_ *= 2;
    }

    cout << "lambda: " << lambda_ << endl;

    for (int i = 0; i < moduleNum; ++i)
    {
        grad_[i].x = wirelengthGrad[i].x + lambda_ * densityGrad[i].x;
        grad_[i].y = wirelengthGrad[i].y + lambda_ * densityGrad[i].y;
    }

    cout << "wirelengthGrad: " << wirelengthGrad[0].x << " " << wirelengthGrad[0].y << endl;
    cout << "densityGrad: " << lambda_ * densityGrad[0].x << " " << lambda_ * densityGrad[0].y << endl;

    iterNum_++;

    return grad_;
}