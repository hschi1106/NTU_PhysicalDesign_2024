#define _GLIBCXX_USE_CXX11_ABI 0 // Align the ABI version to avoid compatibility issues with `Placment.h`
#ifndef OBJECTIVEFUNCTION_H
#define OBJECTIVEFUNCTION_H

#include <vector>

#include "Placement.h"
#include "Point.h"

/**
 * @brief Base class for objective functions
 */
class BaseFunction
{
public:
    /////////////////////////////////
    // Consutructors
    /////////////////////////////////

    BaseFunction(const size_t &input_size) : grad_(input_size) {}

    /////////////////////////////////
    // Accessors
    /////////////////////////////////

    const std::vector<Point2<double>> &grad() const { return grad_; }
    const double &value() const { return value_; }

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    // Forward pass, compute the value of the function
    virtual const double &operator()(const std::vector<Point2<double>> &input) = 0;

    // Backward pass, compute the gradient of the function
    virtual const std::vector<Point2<double>> &Backward() = 0;

protected:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> grad_; // Gradient of the function
    double value_;                     // Value of the function
};

/**
 * @brief Example function for optimization
 *
 * This is a simple example function for optimization. The function is defined as:
 *      f(t) = 3*t.x^2 + 2*t.x*t.y + 2*t.y^2 + 7
 */
class ExampleFunction : public BaseFunction
{
public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////

    ExampleFunction(Placement &placement);

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> input_; // Cache the input for backward pass
    Placement &placement_;
};

/**
 * @brief Wirelength function
 */
class Wirelength : public BaseFunction
{
    // TODO: Implement the wirelength function, add necessary data members for caching
public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////

    Wirelength(Placement &placement);

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

    const double getGamma() const { return gamma_; }

    void setGamma(double gamma) { gamma_ = gamma; }

private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> input_; // Cache the input for backward pass
    double gamma_;
    Placement &placement_;
};

/**
 * @brief Density function
 */
class Density : public BaseFunction
{
    // TODO: Implement the density function, add necessary data members for caching
public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////

    Density(Placement &placement);

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

    const double getOverflowRatio() const { return overflowRatio_; }
    const double getMb() const { return mb_; }
    const int getBinSize() const { return binSize_; }

private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> input_; // Cache the input for backward pass
    Placement &placement_;
    double mb_;         // max density of the bin
    double overflowRatio_; // overflow ratio
    int binSize_;
    int widthBinNum_;
    int heightBinNum_;
    vector<vector<double>> binDensity_;
    vector<vector<double>> binDensityGrad_;
};

/**
 * @brief Objective function for global placement
 */
class ObjectiveFunction : public BaseFunction
{
    // TODO: Implement the objective function for global placement, add necessary data
    // members for caching
    //
    // Hint: The objetive function of global placement is as follows:
    //       f(t) = wirelength(t) + lambda * density(t),
    // where t is the positions of the modules, and lambda is the penalty weight.
    // You may need an interface to update the penalty weight (lambda) dynamically.
public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////
    ObjectiveFunction(Placement &placement)
        : BaseFunction(placement.numModules()), placement_(placement), iterNum_(0), wirelength_(placement), density_(placement)
    {
    }

    /////////////////////////////////
    // Methods
    /////////////////////////////////

    const double &operator()(const std::vector<Point2<double>> &input) override;
    const std::vector<Point2<double>> &Backward() override;

    const double getOverflowRatio() const { return density_.getOverflowRatio(); }
    const double getMb() const { return density_.getMb(); }
    const double getGamma() const { return wirelength_.getGamma(); }
    const int getBinSize() const { return density_.getBinSize(); }

    void setGamma(double gamma) { wirelength_.setGamma(gamma); }

private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    std::vector<Point2<double>> input_; // Cache the input for backward pass
    Placement &placement_;
    int iterNum_;
    double lambda_;
    Wirelength wirelength_;
    Density density_;
};

#endif // OBJECTIVEFUNCTION_H
