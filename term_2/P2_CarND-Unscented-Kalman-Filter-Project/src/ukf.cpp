#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 1.8;//1.8

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 0.6;//0.6

  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;

  ///* initially set to false, set to true in first call of ProcessMeasurement
  is_initialized_ = false;

  ///* time when the state is true, in us
  time_us_ = 0;

  ///* State dimension
  n_x_ = 5;

  ///* Augmented state dimension
  n_aug_ = 7;

  ///* Number of sigma points
  n_sig_ = 2 * n_aug_ + 1;

  ///* Sigma point spreading parameter
  lambda_ = 3 - n_aug_;

  ///* Weights of sigma points
  weights_ = VectorXd(n_sig_);
  double weight_0 = lambda_/(lambda_ + n_aug_);
  weights_(0) = weight_0;
  for (int i=1; i < n_sig_; i++) {  //2n+1 weights
	double weight = 0.5/(lambda_ + n_aug_);
	weights_(i) = weight;
  }

  // initial covariance matrix
  P_.setIdentity();
  P_ = P_ * 0.1;

  ///* predicted sigma points matrix
  Xsig_pred_ = MatrixXd(n_x_, n_sig_);

  //initialize noise measurement covariance matrix R - lidar
  R_lidar_ = MatrixXd(2, 2).setZero();
  R_lidar_(0,0) = std_laspx_ * std_laspx_;
  R_lidar_(1,1) = std_laspy_ * std_laspy_;

  //initialize noise measurement covariance matrix R - radar
  R_radar_ = MatrixXd(3, 3).setZero();
  R_radar_(0,0) = std_radr_ * std_radr_;
  R_radar_(1,1) = std_radphi_ * std_radphi_;
  R_radar_(2,2) = std_radrd_ * std_radrd_;

  // the current NIS for radar
  NIS_radar_  = 0;

  // the current NIS for lidar
  NIS_lidar_ = 0;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {

  if (!is_initialized_) {
  // first measurement
  cout << "UKF: " << endl;

  // initial state vector
  x_ << 1, 1, 0, 0, 0;

  if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
	float rho = meas_package.raw_measurements_[0];
	float phi = meas_package.raw_measurements_[1];
	x_ << rho * cos(phi), rho * sin(phi), 0, 0, 0;
  }
  else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
	x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 0, 0, 0;
  }

  time_us_ = meas_package.timestamp_;
  // done initializing, no need to predict or update
  is_initialized_ = true;
  return;
  }
  ///* Predict

  //compute the time elapsed between the current and previous measurements
  float dt = (meas_package.timestamp_ - time_us_) / 1000000.0;
  time_us_ = meas_package.timestamp_;

  // Call the Kalman Filter predict() function
  Prediction(dt);

  ///* Update

  if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
    UpdateRadar(meas_package);
  } else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
    UpdateLidar(meas_package);
  }

  cout << "x_ = " << endl;
  cout << x_ << endl;
  cout << "P_ = " << endl;
  cout << P_ << endl;
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {

  // 1. Generate Sigma Points
  //create augmented mean vector
  VectorXd x_aug = VectorXd(n_aug_);

  //create augmented state covariance
  MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);

  //create augmented mean state
  x_aug.head(5) = x_;
  x_aug(5) = 0;
  x_aug(6) = 0;

  //create augmented covariance matrix
  P_aug.fill(0.0);
  P_aug.topLeftCorner(5,5) = P_;
  P_aug(5,5) = std_a_*std_a_;
  P_aug(6,6) = std_yawdd_*std_yawdd_;

  //create covariance square root matrix
  MatrixXd Ps = P_aug.llt().matrixL();

  //create augmented sigma points matrix
  MatrixXd Xsig_aug = MatrixXd(n_aug_, n_sig_);
  Xsig_aug.col(0)  = x_aug;
  for (int i = 0; i< n_aug_; i++)
  {
	Xsig_aug.col(i+1)        = x_aug + sqrt(lambda_ + n_aug_) * Ps.col(i);
	Xsig_aug.col(i+1+n_aug_) = x_aug - sqrt(lambda_ + n_aug_) * Ps.col(i);
  }

  // 2. Predict Sigma Points
  for (int i = 0; i < n_sig_; i++)
  {
	double p_x = Xsig_aug(0,i);
	double p_y = Xsig_aug(1,i);
	double v = Xsig_aug(2,i);
	double yaw = Xsig_aug(3,i);
	double yawd = Xsig_aug(4,i);
	double nu_a = Xsig_aug(5,i);
	double nu_yawdd = Xsig_aug(6,i);

	//predicted state values
	double px_p, py_p;

	//avoid division by zero
	if (fabs(yawd) > 0.001) {
		px_p = p_x + v/yawd * (sin(yaw + yawd * delta_t) - sin(yaw));
		py_p = p_y + v/yawd * (cos(yaw) - cos(yaw + yawd * delta_t));
	}
	else {
		px_p = p_x + v * delta_t * cos(yaw);
		py_p = p_y + v * delta_t * sin(yaw);
	}

	double v_p = v;
	double yaw_p = yaw + yawd * delta_t;
	double yawd_p = yawd;

	//add noise
	px_p = px_p + 0.5 * nu_a * delta_t * delta_t * cos(yaw);
	py_p = py_p + 0.5 * nu_a * delta_t * delta_t * sin(yaw);
	v_p = v_p + nu_a * delta_t;

	yaw_p = yaw_p + 0.5 * nu_yawdd * delta_t * delta_t;
	yawd_p = yawd_p + nu_yawdd * delta_t;

	//write predicted sigma point into right column
	Xsig_pred_(0,i) = px_p;
	Xsig_pred_(1,i) = py_p;
	Xsig_pred_(2,i) = v_p;
	Xsig_pred_(3,i) = yaw_p;
	Xsig_pred_(4,i) = yawd_p;
  }

  // 3. Predict mean and covariance

  //create vector for predicted state
  VectorXd x = VectorXd(n_x_).setZero();

  //create covariance matrix for prediction
  MatrixXd P = MatrixXd(n_x_, n_x_).setZero();

  //predicted state mean
  //x_.fill(0.0);
  for (int i = 0; i < n_sig_; i++) {
	x  = x + weights_(i) * Xsig_pred_.col(i);
  }

  //predicted state covariance matrix
  //P_.fill(0.0);
  for (int i = 0; i < n_sig_; i++) {
	// state difference
	VectorXd x_diff = Xsig_pred_.col(i) - x;
	//angle normalization
	while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
	while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;

	P = P + weights_(i) * x_diff * x_diff.transpose() ;
  }
  // 4. Update the state and the covariance matrix
  x_ = x;
  P_ = P;
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {

  //set measurement dimension, lidar can measure px, and py
  int n_z = 2;

  //create matrix for sigma points in measurement space
  MatrixXd Zsig = MatrixXd(n_z, n_sig_);

  //transform sigma points into measurement space
  for (int i = 0; i < n_sig_; ++i) {
	Zsig(0,i) = Xsig_pred_(0,i);
	Zsig(1,i) = Xsig_pred_(1,i);
  }

  //calculate mean predicted measurement
  VectorXd z_pred = VectorXd(n_z).setZero();
  for (int i = 0; i < n_sig_; ++i)
	z_pred = z_pred + weights_(i) * Zsig.col(i);

  //calculate measurement covariance matrix S
  MatrixXd S = MatrixXd(n_z, n_z).setZero();
  for (int i= 0; i < n_sig_; ++i) {
	VectorXd z_diff = Zsig.col(i) - z_pred;
	S = S + weights_(i) * z_diff * z_diff.transpose();
  }
  S = S + R_lidar_;

  //create matrix for cross correlation Tc
  MatrixXd Tc = MatrixXd(n_x_, n_z).setZero();
  //calculate cross correlation matrix
  for (int i = 0; i < n_sig_; ++i)
  {
	VectorXd x_diff = Xsig_pred_.col(i) - x_;
	VectorXd z_diff = Zsig.col(i) - z_pred;
	Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
  }

  //calculate Kalman gain K;
  MatrixXd K = Tc * S.inverse();

  //measurements from radar
  VectorXd z = meas_package.raw_measurements_;

  //update state mean and covariance matrix
  VectorXd z_var = z - z_pred;

  x_ = x_ + K * z_var;

  P_ = P_ - K * S * K.transpose();

  NIS_lidar_ = z_var.transpose() * S * z_var;
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {

  //set measurement dimension, radar can measure r, phi, and r_dot
  int n_z = 3;

  //create matrix for sigma points in measurement space
  MatrixXd Zsig = MatrixXd(n_z, n_sig_);

  //transform sigma points into measurement space
  for (int i = 0; i < n_sig_; ++i) {
	double px, py, v, psi, psi_dot = 0;
	px = Xsig_pred_(0,i);
	py = Xsig_pred_(1,i);
	v = Xsig_pred_(2,i);
	psi = Xsig_pred_(3,i);
	psi_dot = Xsig_pred_(4,i);

	double rho, phi, rho_dot = 0;
	rho = sqrt(px*px + py*py);
	phi = atan2(py, px);
	if (fabs(rho) < 0.0001)
	  rho_dot = 0;
	else
	  rho_dot = (px * v * cos(psi) + py * v * sin(psi))/rho;
	Zsig(0,i) = rho;
	Zsig(1,i) = phi;
	Zsig(2,i) = rho_dot;
  }

  //calculate mean predicted measurement
  VectorXd z_pred = VectorXd(n_z).setZero();
  for (int i = 0; i < n_sig_; ++i)
	z_pred = z_pred + weights_(i) * Zsig.col(i);

  //calculate measurement covariance matrix S
  MatrixXd S = MatrixXd(n_z, n_z).setZero();
  for (int i= 0; i < n_sig_; ++i) {
	VectorXd z_diff = Zsig.col(i) - z_pred;
	while (z_diff(1) > M_PI) z_diff(1) -= 2.*M_PI;
	while (z_diff(1) < -M_PI) z_diff(1) += 2.*M_PI;
	S = S + weights_(i) * z_diff * z_diff.transpose();
  }
  S = S + R_radar_;

  //create matrix for cross correlation Tc
  MatrixXd Tc = MatrixXd(n_x_, n_z).setZero();

  //calculate cross correlation matrix
  for (int i = 0; i < n_sig_; ++i)
  {
	  VectorXd x_diff = Xsig_pred_.col(i) - x_;
	  while (x_diff(3) > M_PI) x_diff(3) -= 2.*M_PI;
	  while (x_diff(3) < -M_PI) x_diff(3) += 2.*M_PI;

	  VectorXd z_diff = VectorXd(n_z).setZero();
	  z_diff = Zsig.col(i) - z_pred;
	  while (z_diff(1) > M_PI) z_diff(1) -= 2.*M_PI;
	  while (z_diff(1) < -M_PI) z_diff(1) += 2.*M_PI;

	  Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
  }

  //calculate Kalman gain K;
  MatrixXd K = Tc * S.inverse();

  //measurements from radar
  VectorXd z = meas_package.raw_measurements_;

  //update state mean and covariance matrix
  VectorXd z_var = z - z_pred;
  while (z_var(1) > M_PI) z_var(1) -= 2.*M_PI;
  while (z_var(1) < -M_PI) z_var(1) += 2.*M_PI;

  x_ = x_ + K * z_var;

  P_ = P_ - K * S * K.transpose();

  NIS_radar_ = z_var.transpose() * S.inverse() * z_var;
}