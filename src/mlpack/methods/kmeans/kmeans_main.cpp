/**
 * @file kmeans_main.cpp
 * @author Ryan Curtin
 *
 * Executable for running K-Means.
 */
#include <mlpack/core.hpp>

#include "kmeans.hpp"
#include "allow_empty_clusters.hpp"

using namespace mlpack;
using namespace mlpack::kmeans;
using namespace std;

// Define parameters for the executable.
PROGRAM_INFO("K-Means Clustering", "This program performs K-Means clustering "
    "on the given dataset, storing the learned cluster assignments either as "
    "a column of labels in the file containing the input dataset or in a "
    "separate file.  Empty clusters are not allowed by default; when a cluster "
    "becomes empty, the point furthest from the centroid of the cluster with "
    "maximum variance is taken to fill that cluster.", "");

PARAM_STRING_REQ("input_file", "Input dataset to perform clustering on.", "");
PARAM_INT_REQ("clusters", "Number of clusters to find.", "");
PARAM_FLAG("in_place", "If specified, a column of the learned cluster "
    "assignments will be added to the input dataset file.  In this case "
    "--output_file is not necessary.", "");
PARAM_STRING("output_file", "File to write output labels to.", "", "");
PARAM_FLAG("allow_empty_clusters", "Allow empty clusters to be created.", "");
PARAM_FLAG("labels_only", "Only output labels into output file.", "");
PARAM_DOUBLE("overclustering", "Finds (overclustering * clusters) clusters, "
    "then merges them together until only the desired number of clusters are "
    "left.", "", 1.0);
PARAM_INT("max_iterations", "Maximum number of iterations before K-Means "
    "terminates.", "", 1000);

int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);

  // Initialize random seed -- because that makes a difference.
  srand(time(NULL));

  // Now do validation of options.
  string input_file = CLI::GetParam<string>("input_file");
  int clusters = CLI::GetParam<int>("clusters");
  if (clusters < 1)
  {
    Log::Fatal << "Invalid number of clusters requested (" << clusters << ")! "
        << "Must be greater than or equal to 1." << std::endl;
  }

  int maxIterations = CLI::GetParam<int>("max_iterations");
  if (maxIterations < 0)
  {
    Log::Fatal << "Invalid value for maximum iterations (" << maxIterations <<
        ")! Must be greater than or equal to 0." << std::endl;
  }

  double overclustering = CLI::GetParam<double>("overclustering");
  if (overclustering < 1)
  {
    Log::Fatal << "Invalid value for overclustering (" << overclustering <<
        ")! Must be greater than or equal to 1." << std::endl;
  }

  // Make sure we have an output file if we're not doing the work in-place.
  if (!CLI::HasParam("in_place") && !CLI::HasParam("output_file"))
  {
    Log::Fatal << "--output_file not specified (and --in_place not set)."
        << std::endl;
  }

  // Load our dataset.
  arma::mat dataset;
  data::Load(input_file.c_str(), dataset);

  // Now create the KMeans object.  Because we could be using different types,
  // it gets a little weird...
  arma::Col<size_t> assignments;
  if (CLI::HasParam("allow_empty_clusters"))
  {
    KMeans<metric::SquaredEuclideanDistance, RandomPartition,
        AllowEmptyClusters> k(maxIterations, overclustering);

    k.Cluster(dataset, clusters, assignments);
  }
  else
  {
    KMeans<> k(maxIterations, overclustering);

    k.Cluster(dataset, clusters, assignments);
  }

  // Now figure out what to do with our results.
  if (CLI::HasParam("in_place"))
  {
    // Add the column of assignments to the dataset; but we have to convert them
    // to type double first.
    arma::vec converted(assignments.n_elem);
    for (size_t i = 0; i < assignments.n_elem; i++)
      converted(i) = (double) assignments(i);

    dataset.insert_rows(dataset.n_rows, trans(converted));

    // Save the dataset.
    data::Save(input_file.c_str(), dataset);
  }
  else
  {
    if (CLI::HasParam("only_labels"))
    {
      // Save only the labels.
      string output_file = CLI::GetParam<string>("output_file");
      arma::Mat<size_t> output = trans(assignments);
      data::Save(output_file.c_str(), output);
    }
    else
    {
      // Convert the assignments to doubles.
      arma::vec converted(assignments.n_elem);
      for (size_t i = 0; i < assignments.n_elem; i++)
        converted(i) = (double) assignments(i);

      dataset.insert_rows(dataset.n_rows, trans(converted));

      // Now save, in the different file.
      string output_file = CLI::GetParam<string>("output_file");
      data::Save(output_file.c_str(), dataset);
    }
  }
}
