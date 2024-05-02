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
        double inputX = input[i].x - placement_.boundryLeft();
        double inputY = input[i].y - placement_.boundryBottom();
        posExpValue_[i].x = exp(inputX / gamma);
        posExpValue_[i].y = exp(inputY / gamma);
        negExpValue_[i].x = exp(-inputX / gamma);
        negExpValue_[i].y = exp(-inputY / gamma);
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

Density::Density(Placement &placement, int binNumPerEdge) : BaseFunction(placement.numModules()), placement_(placement), constA_(1), binNumPerEdge_(binNumPerEdge)
{
    // resize the vector to store the density and gradient
    binDensity_.resize(binNumPerEdge);
    binDensityGrad_.resize(binNumPerEdge);
    for (int i = 0; i < binNumPerEdge; ++i)
    {
        binDensity_[i].resize(binNumPerEdge);
        binDensityGrad_[i].resize(binNumPerEdge);
        for (int j = 0; j < binNumPerEdge; ++j)
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
    mb_ = (double)totalArea / ((placement.boundryRight() - placement.boundryLeft()) * (placement.boundryTop() - placement.boundryBottom()));
}

double sigmoid(int l, int x, int u)
{
    return 1 / (1 + exp(-(x - l))) / (1 + exp(-(u - x)));
}

const double &Density::operator()(const std::vector<Point2<double>> &input)
{
    // Compute the value of the density function

    // Initialize the value of the density function
    value_ = 0;
    overflowRatio_ = 0;
    for (int i = 0; i < binNumPerEdge_; ++i)
    {
        for (int j = 0; j < binNumPerEdge_; ++j)
        {
            binDensity_[i][j] = 0;
        }
    }
    int moduleNum = placement_.numModules();
    double binWidth = (placement_.boundryRight() - placement_.boundryLeft()) / binNumPerEdge_;
    double binHeight = (placement_.boundryTop() - placement_.boundryBottom()) / binNumPerEdge_;
    for (int i = 0; i < moduleNum; ++i)
    {
        int startBinX = (input[i].x - placement_.boundryLeft()) / binWidth < 0 ? 0 : (input[i].x - placement_.boundryLeft()) / binWidth;
        int endBinX = (input[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binWidth >= binNumPerEdge_ ? binNumPerEdge_ - 1 : (input[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binWidth;
        int startBinY = (input[i].y - placement_.boundryBottom()) / binHeight < 0 ? 0 : (input[i].y - placement_.boundryBottom()) / binHeight;
        int endBinY = (input[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binHeight >= binNumPerEdge_ ? binNumPerEdge_ - 1 : (input[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binHeight; 
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                double inputX = input[i].x - placement_.boundryLeft() + moduleWidth - j * binWidth;
                double inputY = input[i].y - placement_.boundryBottom() + moduleHeight - k * binHeight;
                // assert j, k in the range
                assert(j >= 0 && j < binNumPerEdge_);
                assert(k >= 0 && k < binNumPerEdge_);
                binDensity_[j][k] += sigmoid(-moduleWidth, inputX, moduleWidth) * sigmoid(-moduleHeight, inputY, moduleHeight);
            }
        }
    }

    for (int i = 0; i < binNumPerEdge_; ++i)
    {
        for (int j = 0; j < binNumPerEdge_; ++j)
        {
            value_ += (binDensity_[i][j] - mb_) * (binDensity_[i][j] - mb_);
            overflowRatio_ += binDensity_[i][j] > mb_ ? binDensity_[i][j] - mb_ : 0;
        }
    }

    overflowRatio_ /= binNumPerEdge_ * binNumPerEdge_;

    input_ = input;

    return value_;
}

double sigmoidDrivative(int l, int x, int u)
{
    if (abs(x - l) > 500 && abs(u - x) > 500)
    {
        return 0;
    }
    double exp1 = exp(-(x - l));
    double exp2 = exp(-(u - x));
    return (exp1 - exp2) / (1 + exp1) / (1 + exp2) / (1 + exp1) / (1 + exp2);
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
    double binWidth = (placement_.boundryRight() - placement_.boundryLeft()) / binNumPerEdge_;
    double binHeight = (placement_.boundryTop() - placement_.boundryBottom()) / binNumPerEdge_;
    for (int i = 0; i < moduleNum; ++i)
    {
        int startBinX = (input_[i].x - placement_.boundryLeft()) / binWidth < 0 ? 0 : (input_[i].x - placement_.boundryLeft()) / binWidth;
        int endBinX = (input_[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binWidth >= binNumPerEdge_ ? binNumPerEdge_ - 1 : (input_[i].x + placement_.module(i).width() - placement_.boundryLeft()) / binWidth;
        int startBinY = (input_[i].y - placement_.boundryBottom()) / binHeight < 0 ? 0 : (input_[i].y - placement_.boundryBottom()) / binHeight;
        int endBinY = (input_[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binHeight >= binNumPerEdge_ ? binNumPerEdge_ - 1 : (input_[i].y + placement_.module(i).height() - placement_.boundryBottom()) / binHeight; 
        double moduleWidth = placement_.module(i).width(), moduleHeight = placement_.module(i).height();
        for (int j = startBinX; j <= endBinX; ++j)
        {
            for (int k = startBinY; k <= endBinY; ++k)
            {
                double inputX = input_[i].x - placement_.boundryLeft() + moduleWidth - j * binWidth;
                double inputY = input_[i].y - placement_.boundryBottom() + moduleHeight - k * binHeight;
                double sigmoidX = sigmoid(-moduleWidth, inputX, moduleWidth);
                double sigmoidY = sigmoid(-moduleHeight, inputY, moduleHeight);
                double sigmoidXDrivative = sigmoidDrivative(-moduleWidth, inputX, moduleWidth);
                double sigmoidYDrivative = sigmoidDrivative(-moduleHeight, inputY, moduleHeight);
                grad_[i].x += 2 * (binDensity_[j][k] - mb_) * sigmoidXDrivative * sigmoidY;
                grad_[i].y += 2 * (binDensity_[j][k] - mb_) * sigmoidX * sigmoidYDrivative;
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
    iterNum_++;
    vector<Point2<double>> wirelengthGrad, densityGrad;
    wirelengthGrad = wirelength_.Backward();
    densityGrad = density_.Backward();
    int moduleNum = placement_.numModules();

    if (iterNum_ == 1)
    {
        double wirelengthGradNorm = 0, densityGradNorm = 0;
        for (int i = 0; i < moduleNum; ++i)
        {
            wirelengthGradNorm += sqrt(wirelengthGrad[i].x * wirelengthGrad[i].x + wirelengthGrad[i].y * wirelengthGrad[i].y);
            densityGradNorm += sqrt(densityGrad[i].x * densityGrad[i].x + densityGrad[i].y * densityGrad[i].y);
        }
        lambda_ = wirelengthGradNorm / densityGradNorm;
        cout << "calculate lambda: " << lambda_ << endl;
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

    return grad_;
}