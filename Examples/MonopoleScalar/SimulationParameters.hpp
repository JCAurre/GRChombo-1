/* GRChombo
 * Copyright 2012 The GRChombo collaboration.
 * Please refer to LICENSE in GRChombo's root directory.
 */

#ifndef SIMULATIONPARAMETERS_HPP_
#define SIMULATIONPARAMETERS_HPP_

// General includes
#include "GRParmParse.hpp"
#include "SimulationParametersBase.hpp"

// Problem specific includes:
#include "InitialScalarData.hpp"
#include "KerrBH.hpp"
#include "Potential.hpp"

// morris: reading initial data from text file and storing contents in an array (files phi.txt and r.txt)
#include <iostream>
#include <fstream>
#include <vector>

class SimulationParameters : public SimulationParametersBase
{
  public:
    SimulationParameters(GRParmParse &pp) : SimulationParametersBase(pp)
    {
        // read the problem specific params
        read_params(pp);
        check_params();
    }

    void read_params(GRParmParse &pp)
    {
        // Initial scalar field data
        initial_params.center =
            center; // already read in SimulationParametersBase
        pp.load("G_Newton", G_Newton,
                0.0); // for now the example neglects backreaction
        pp.load("scalar_amplitude", initial_params.amplitude, 0.1);
        pp.load("scalar_width", initial_params.width, 1.0);
        pp.load("scalar_mass", potential_params.scalar_mass, 0.1);

        // Initial Kerr data
        pp.load("kerr_mass", kerr_params.mass, 1.0);
        pp.load("kerr_spin", kerr_params.spin, 0.0);
        pp.load("kerr_center", kerr_params.center, center);

        // morris: adding parameters for potential
        double eta = 0.07;
        pp.load("lambda", potential_params.lambda, 1.0);
        pp.load("eta", potential_params.eta, eta);
        pp.load("initial_eta", initial_params.eta, eta); // for InitialScalarData.hpp

        // morris: adding initial potential parameters
        // LOADING IN INITIAL DATA INTO ARRAY:
        // open file for reading
        ifstream initial_f_file("flatspace_initial_f_eta7e-2.txt");
        // determine number of elements in the file
        int num_elements;
        initial_f_file >> num_elements;
        // create an array of the desired size
        vector<float> initial_f(num_elements);
        // read initial f values in
        for (int i = 0; i < num_elements; ++i) {
            initial_f_file >> initial_f[i];
        }
        // close the file
        initial_f_file.close();

        // doing the same for the r value
        ifstream initial_r_file("flatspace_initial_r_eta7e-2.txt");
        vector<float> initial_r(num_elements);
        for (int i = 0; i < num_elements; ++i) {
            initial_r_file >> initial_r[i];
        }
        initial_r_file.close();


        // QUESTION 1: get error "no matching member function for call to 'load'" as third argument is normally an int
        // However I need this to be a vector
        pp.load("initial_f", initial_params.initial_f, initial_f);
        pp.load("initial_r", initial_params.initial_r, initial_r);

    }

    void check_params()
    {
        warn_parameter("scalar_mass", potential_params.scalar_mass,
                       potential_params.scalar_mass <
                           0.2 / coarsest_dx / dt_multiplier,
                       "oscillations of scalar field do not appear to be "
                       "resolved on coarsest level");
        warn_parameter("scalar_width", initial_params.width,
                       initial_params.width < 0.5 * L,
                       "is greater than half the domain size");
        warn_parameter("kerr_mass", kerr_params.mass, kerr_params.mass >= 0.0,
                       "should be >= 0.0");
        check_parameter("kerr_spin", kerr_params.spin,
                        std::abs(kerr_params.spin) <= kerr_params.mass,
                        "must satisfy |a| <= M = " +
                            std::to_string(kerr_params.mass));
        FOR(idir)
        {
            std::string name = "kerr_center[" + std::to_string(idir) + "]";
            warn_parameter(
                name, kerr_params.center[idir],
                (kerr_params.center[idir] >= 0) &&
                    (kerr_params.center[idir] <= (ivN[idir] + 1) * coarsest_dx),
                "should be within the computational domain");
        }
    }

    // Initial data for matter and potential and BH
    double G_Newton;
    InitialScalarData::params_t initial_params;
    Potential::params_t potential_params;
    KerrBH::params_t kerr_params;
};

#endif /* SIMULATIONPARAMETERS_HPP_ */