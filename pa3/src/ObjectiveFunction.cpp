#include "ObjectiveFunction.h"

#include "cstdio"

#include "cassert"

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

const double &Wirelength::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the wirelength function
    double gamma = max(placement_.boundryRight() - placement_.boundryLeft(), placement_.boundryTop() - placement_.boundryBottom()) / 100;
    value_ = 0; // Initialize the value of the wirelength function
    int netNum = placement_.numNets();
    int moduleNum = placement_.numModules();

    // resize the vector to store the exp value
    posExpValue_.resize(input.size());
    negExpValue_.resize(input.size());

    for (int i = 0; i < moduleNum; ++i)
    {
        posExpValue_[i].x = exp(input[i].x / gamma);
        posExpValue_[i].y = exp(input[i].y / gamma);
        negExpValue_[i].x = exp(-input[i].x / gamma);
        negExpValue_[i].y = exp(-input[i].y / gamma);
    }
    for (int i = 0; i < netNum; ++i)
    {
        double expMaxX = 0, expMinX = 0, expMaxY = 0, expMinY = 0;
        Net net = placement_.net(i);
        int pinNum = net.numPins();
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            expMaxX += posExpValue_[id].x;
            expMinX += negExpValue_[id].x;
            expMaxY += posExpValue_[id].y;
            expMinY += negExpValue_[id].y;
        }
        value_ += log(expMaxX) + log(expMinX) + log(expMaxY) + log(expMinY);
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
        double sigmaPosExpX = 0, sigmaNegExpX = 0, sigmaPosExpY = 0, sigmaNegExpY = 0;
        Net net = placement_.net(i);
        int pinNum = net.numPins();
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            sigmaPosExpX += posExpValue_[id].x;
            sigmaNegExpX += negExpValue_[id].x;
            sigmaPosExpY += posExpValue_[id].y;
            sigmaNegExpY += negExpValue_[id].y;
        }
        for (int j = 0; j < pinNum; ++j)
        {
            Pin pin = net.pin(j);
            int id = pin.moduleId();
            grad_[id].x += posExpValue_[id].x / sigmaPosExpX - negExpValue_[id].x / sigmaNegExpX;
            grad_[id].y += posExpValue_[id].y / sigmaPosExpY - negExpValue_[id].y / sigmaNegExpY;
        }
    }

    return grad_;
}

Density::Density(Placement &placement) : BaseFunction(placement.numModules()), placement_(placement), binSize_(50)
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
    int totalArea = 0, moduleNum = placement.numModules();
    for (int i = 0; i < moduleNum; ++i)
    {
        totalArea += placement.module(i).width() * placement.module(i).height();
    }
    mb_ = (double)totalArea / (outLineWidth * outLineHeight);
}

const double &Density::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the density function

    // Initialize the value of the density function
    cout << "start" << endl;

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
                double inputX = input[i].x - placement_.boundryLeft() + moduleWidth - j * binSize_;
                double inputY = input[i].y - placement_.boundryBottom() + moduleHeight - k * binSize_;
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
            overflowRatio_ += binDensity_[i][j] > mb_ ? binDensity_[i][j] - mb_ : 0;
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
                double inputX = input_[i].x - placement_.boundryLeft() + moduleWidth - j * binSize_;
                double inputY = input_[i].y - placement_.boundryBottom() + moduleHeight - k * binSize_;
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
                        dPx = 2 * bx * (-inputX - moduleWidth / 2 - 2 * binSize_);
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
                        dPy = 2 * by * (-inputY - moduleHeight / 2 - 2 * binSize_);
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
        lambda_ = wirelengthGradNorm / densityGradNorm;
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