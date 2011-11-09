#include <mlpack/core.h>
#include <mlpack/methods/linear_regression/linear_regression.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(LinearRegressionTest);

  /**
   * Creates two 10x3 random matrices and one 10x1 "results" matrix.
   * Finds B in y=BX with one matrix, then predicts against the other.
   */
  BOOST_AUTO_TEST_CASE(LinearRegressionTest)
  {
    // Predictors and points are 100x3 matrices.
    arma::mat predictors(3, 10);
    arma::mat points(3, 10);

    // Responses is the "correct" value for each point in predictors and points.
    arma::colvec responses(10);

    // The values we get back when we predict for points.
    arma::rowvec predictions(10);

    // We'll randomly select some coefficients for the linear response.
    arma::vec coeffs;
    coeffs.randu(4);
    coeffs(0) = 0;

    // Now generate each point.
    for (size_t row = 0; row < 3; row++)
      predictors.row(row) = arma::linspace<arma::rowvec>(0, 9, 10);

    points = predictors;

    // Now add a small amount of noise to each point.
    for (size_t elem = 0; elem < points.n_elem; elem++)
    {
      // Max added noise is 0.02.
      points[elem] += ((double) rand() / (double) INT_MAX) / 50.0;
      predictors[elem] += ((double) rand() / (double) INT_MAX) / 50.0;
    }

    // Generate responses.
    for (size_t elem = 0; elem < responses.n_elem; elem++)
      responses[elem] = coeffs[0] +
          dot(coeffs.rows(1, 3), arma::ones<arma::rowvec>(3) * elem);

    // Initialize and predict
    mlpack::linear_regression::LinearRegression lr(predictors, responses);
    lr.predict(predictions, points);

    // Output result and verify we have less than 5% error from "correct" value
    // for each point
    std::cout << "Actual model:\n" << coeffs << '\n';
    std::cout << "Parameters:\n" << lr.getParameters() << '\n';
    std::cout << "Predictors:\n" << predictors << '\n';
    std::cout << "Points:\n" << points << '\n' << "Predictions:\n"
        << predictions << '\n';
    std::cout << "Correct:\n" << responses << '\n';
    for(size_t i = 0; i < predictions.n_cols; ++i)
      BOOST_REQUIRE_CLOSE(predictions(i), responses(i), 5);
  }

BOOST_AUTO_TEST_SUITE_END();