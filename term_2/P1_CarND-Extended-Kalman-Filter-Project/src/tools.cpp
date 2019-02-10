#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {

	VectorXd rmse(4);
	rmse << 0,0,0,0;
	// Validity of vectors' size
	if(estimations.size() != ground_truth.size() || estimations.size() == 0){
		std::cout << "Invalid estimation or ground_truth data" << std::endl;
		return rmse;
	}

	// Store squared residuals
	for (size_t i = 0; i < estimations.size(); ++i){

		VectorXd residual = estimations[i] - ground_truth[i];

		// Square values through element-wise multiplication
		residual = residual.array() * residual.array();
		rmse += residual;
	}

	// Mean calculation
	rmse = rmse / estimations.size();

	// Square root calculation
	rmse = rmse.array().sqrt();

	return rmse;
}

MatrixXd Tools::CalculateJacobian(const VectorXd& x_state) {

	MatrixXd Hj(3,4);

	// Allocate state parameters
	float px = x_state(0);
	float py = x_state(1);
	float vx = x_state(2);
	float vy = x_state(3);

	// Pre-computation of parameters for fast calculations
	float inroot = px*px + py*py;
	float den = sqrt(inroot);
	float den2 = inroot*den;

	// Check division by zero
	if(fabs(inroot) < 0.0001){
		std::cout << "CalculateJacobian () - Error - Division by Zero" << std::endl;
		return Hj;
	}

	// Compute Jacobian matrix
	Hj << px/den, py/den, 0, 0,
		  -py/inroot, px/inroot, 0, 0,
		  py*(vx*py - vy*px)/den2, px*(px*vy - py*vx)/den2, px/den, py/den;

	return Hj;
}
