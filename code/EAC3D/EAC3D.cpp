// EAC3D.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "EAC3D.h"
#include <iostream>
#include <vector>



void EAC3D::set_NBEquations(int NBEQ) {
	this->DIM_ARRAY = NBEQ;
}

EAC3D::EAC3D(double Lx, double Ly, double Lz, int nx, int ny, int nz) {

	this->DIM_ARRAY = 1;
	this->isFirstStep = true;
	this->innerRK2Step = 0;
	this->x0 = 0.;
	this->y0 = 0.;
	this->z0 = 0.;


	this->time = 0.0;
	
	this->TimeVector.push_back(time);
	this->Lx = Lx;
	this->Ly = Ly;
	this->Lz = Lz;


	this->nx = nx ;
	this->ny = ny ;
	this->nz = nz ;


	this->dx = this->Lx / double(this->nx-1);
	this->dy = this->Ly / double(this->ny-1);
	this->dz = this->Lz / double(this->nz-1);

	this->Strain = new std::vector<double>**[this->nx];
	this->Stress = new std::vector<double>**[this->nx];
	for (int i = 0; i < this->nx; i++) {
		this->Strain[i] = new std::vector<double>*[this->ny];
		this->Stress[i] = new std::vector<double>*[this->ny];
		for (int j = 0; j < this->ny; j++) {
			this->Strain[i][j] = new std::vector<double>[this->nz];
			this->Stress[i][j] = new std::vector<double>[this->nz];

			for (int k = 0; k < this->nz; k++) {
				this->Strain[i][j][k].push_back(0.0);
				this->Stress[i][j][k].push_back(0.0);
			}
		}
	}

	this->F = new double***[DIM_ARRAY];
	this->Finit = new double***[DIM_ARRAY];
	this->Fstar = new double***[DIM_ARRAY];

	for (int d = 0; d < DIM_ARRAY; d++) {
		this->F[d] = new double**[this->nx];
		this->Finit[d] = new double**[this->nx];
		this->Fstar[d] = new double**[this->nx];

		for (int i = 0; i < this->nx; i++) {
			this->F[d][i] = new double*[this->ny];
			this->Finit[d][i] = new double*[this->ny];
			this->Fstar[d][i] = new double*[this->ny];

			for (int j = 0; j < this->ny; j++) {
				this->F[d][i][j] = new double[this->nz];
				this->Finit[d][i][j] = new double[this->nz];
				this->Fstar[d][i][j] = new double[this->nz];
			}
		}
	}


	this->RHSnm1 = new double***[DIM_ARRAY];
	for (int d = 0; d < DIM_ARRAY; d++) {
		this->RHSnm1[d] = new double**[this->nx];
		for (int i = 0; i < this->nx; i++) {
			this->RHSnm1[d][i] = new double*[this->ny];
			for (int j = 0; j < this->ny; j++) {
				this->RHSnm1[d][i][j] = new double[this->nz];
			}
		}
	}

	this->RHS = new double***[DIM_ARRAY];
	for (int d = 0; d < DIM_ARRAY; d++) {
		this->RHS[d] = new double**[this->nx];
		for (int i = 0; i < this->nx; i++) {
			this->RHS[d][i] = new double*[this->ny];
			for (int j = 0; j < this->ny; j++) {
				this->RHS[d][i][j] = new double[this->nz];
			}
		}
	}

	this->DCoef = new func_ptr_dd[DIM_ARRAY];
}


EAC3D::~EAC3D() {


	for (int d = 0; d < DIM_ARRAY; d++) {
		for (int i = 0; i < this->nx; i++) {
			for (int j = 0; j < this->ny; j++) {
				delete[] this->RHS[d][i][j];
			}
			delete[] this->RHS[d][i];
		}
		delete[] this->RHS[d];
	}
	delete[] this->RHS;


	for (int d = 0; d < DIM_ARRAY; d++) {
		for (int i = 0; i < this->nx; i++) {
			for (int j = 0; j < this->ny; j++) {
				delete[] this->F[d][i][j];
			}
			delete[] this->F[d][i];
		}
		delete[] this->F[d];
	}
	delete[] this->F;

	for (int d = 0; d < DIM_ARRAY; d++) {
		for (int i = 0; i < this->nx; i++) {
			for (int j = 0; j < this->ny; j++) {
				delete[] this->Finit[d][i][j];
			}
			delete[] this->Finit[d][i];
		}
		delete[] this->Finit[d];
	}
	delete[] this->Finit;


	for (int d = 0; d < DIM_ARRAY; d++) {
		for (int i = 0; i < this->nx; i++) {
			for (int j = 0; j < this->ny; j++) {
				delete[] this->Fstar[d][i][j];
			}
			delete[] this->Fstar[d][i];
		}
		delete[] this->Fstar[d];
	}
	delete[] this->Fstar;

	for (int d = 0; d < DIM_ARRAY; d++) {
		for (int i = 0; i < this->nx; i++) {
			for (int j = 0; j < this->ny; j++) {
				delete[] this->RHSnm1[d][i][j];
			}
			delete[] this->RHSnm1[d][i];
		}
		delete[] this->RHSnm1[d];
	}
	delete[] this->RHSnm1;

	for (int i = 0; i < this->nx; i++) {
		for (int j = 0; j < this->ny; j++) {
			for (int k = 0; k < this->nz; k++) {
				Strain[i][j][k].clear();
				Stress[i][j][k].clear();
			}
			delete[] Strain[i][j];
			delete[] Stress[i][j];
		}
		delete[] Strain[i];
		delete[] Stress[i];
	}
	delete[] Strain;
	delete[] Stress;


	delete[] this->DCoef;
}


double EAC3D::set_dt(double ldt) {
	this->dt = ldt;
	return this->dt;
}

void EAC3D::computeStrainStress() {
	
	int d = 0;
	double x, y, z;
	double t = this->TimeVector.back();


	

	for (int i = 0; i < this->nx; i++) {
		x = x0 + i * this->dx;
		for (int j = 0; j < ny; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz; k++) {
				z = z0 + k * this->dz;
				double dstrain_ = 0.0;
				int size = Strain[i][j][k].size();
				double tprime, K;
				//dstrain_ = (*this->DStrainODT)(F[d][i][j][k]) * (F[d][i][j][k] - Finit[d][i][j][k]);			
				dstrain_ = (*this->DStrainODT)(F[d][i][j][k]) - (*this->DStrainODT)(Finit[d][i][j][k]);
				Strain[i][j][k].push_back( dstrain_);

				double stress_ = 0.0;
				K = (*Restrain_func)(x, y, z);
				size = Strain[i][j][k].size();
				if (size > 1) {
					for (int l = 1; l < size; l++) {
						tprime = this->TimeVector.at(l);
						stress_ += -K * (Strain[i][j][k][size - 1] - Strain[i][j][k][size - 2]) / J_relax(t, tprime);
					}
				}
				Stress[i][j][k].push_back(stress_);
			}
		}
	}
}

void EAC3D::implicitTimeMarching(double CN_factor, int nbIter)
{
	int d = 0;

	/*initializing*/
	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < ny; j++) {
			for (int k = 0; k < nz; k++) {
				Fstar[d][i][j][k] = F[d][i][j][k];
			}
		}
	}

	

	while (nbIter) {
		std::vector<double> accList = std::vector<double>();
		std::vector<int> colList = std::vector<int>();

		int nbEqs = nx * ny*nz;
		double *b = new double[nbEqs];

		int * rowptr = new int[nbEqs+1];


		func_ptr_dd tmp;

		int IDRow, IDCol , ID, iterRow;
		double Fx, Fy, Fz;
		double val, val_b;

		nbIter--;
		ID = 0;
		iterRow = 0;
		int ii, jj, kk;
		tmp = this->DCoef[d];
		for (int i = 0; i < nx; i++) {
			for (int j = 0; j < ny; j++) {
				for (int k = 0; k < nz; k++) {
					IDRow = (i*ny + j)*nz + k;
					//rowList.push_back(ID);
					rowptr[iterRow] = ID;
					iterRow++;
					double x, y, z;
					if (i == 0) {
						switch (this->type_bnd_T_xs) {
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							y = y0 + j * this->dy;
							z = z0 + k * this->dz;

							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = (*bnd_T_xs)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor*(*bnd_T_xs)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] +=(1-CN_factor)* (*bnd_T_xs)(y, z, time , this->F[d][i][j][k]);
							break;
						case BND_NEU:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							y = y0 + j * this->dy;
							z = z0 + k * this->dz;

							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = dx / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_xs)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* dx / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_xs)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor) * dx / (*tmp)(this->F[d][i][j][k])* (*bnd_T_xs)(y, z, time, this->F[d][i][j][k]);
							ii = i + 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;

							
							ii = i + 2;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							
							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else if (i == nx - 1) {
						switch (this->type_bnd_T_xf) {
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							y = y0 + j * this->dy;
							z = z0 + k * this->dz;
							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = (*bnd_T_xf)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor*(*bnd_T_xf)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1 - CN_factor) * (*bnd_T_xf)(y, z, time, this->F[d][i][j][k]);
							break;
						case BND_NEU:
							ii = i - 2;
							jj = j;
							kk = k;
							
							IDCol = (ii*ny + jj)*nz + kk;
							y = y0 + j * this->dy;
							z = z0 + k * this->dz;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							
							ii = i - 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;


							ii = i;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = dx / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_xf)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* dx / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_xf)(y, z, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor) * dx / (*tmp)(this->F[d][i][j][k])* (*bnd_T_xf)(y, z, time, this->F[d][i][j][k]);

							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else if (j == 0) {
						switch (this->type_bnd_T_ys) {
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							z = z0 + k * this->dz;
							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = (*bnd_T_ys)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* (*bnd_T_ys)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor) * (*bnd_T_ys)(z, x, time , this->F[d][i][j][k]);
							break;
						case BND_NEU:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							z = z0 + k * this->dz;
							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = dy / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_ys)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor*dy / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_ys)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] +=(1-CN_factor)* dy / (*tmp)(this->F[d][i][j][k])* (*bnd_T_ys)(z, x, time, this->F[d][i][j][k]);

							jj = j + 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;
							
							jj = j + 2;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							
							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else if (j == ny - 1) {
						switch (this->type_bnd_T_yf) {
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							z = z0 + k * this->dz;

							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							
							b[IDRow] = (*bnd_T_yf)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor *(*bnd_T_yf)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor)*(*bnd_T_yf)(z, x, time, this->F[d][i][j][k]);
							break;
						case BND_NEU:
							
							ii = i;
							jj = j - 2;
							kk = k;
							
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							z = z0 + k * this->dz;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							
							jj = j - 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;

							jj = j;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] = dy / (*tmp)(this->Fstar[d][i][j][k]) * (*bnd_T_yf)(z, x, time + dt, this->Fstar[d][i][j][k]);
							
							//b[IDRow] = CN_factor* dy / (*tmp)(this->Fstar[d][i][j][k]) * (*bnd_T_yf)(z, x, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor)* dy / (*tmp)(this->F[d][i][j][k]) * (*bnd_T_yf)(z, x, time, this->F[d][i][j][k]);

							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else if (k == 0) {
						switch (this->type_bnd_T_zs)
						{
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							y = y0 + j * this->dy;

							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] =  (*bnd_T_zs)(x, y, time + dt, this->Fstar[d][i][j][k]);
							
							//b[IDRow] = CN_factor* (*bnd_T_zs)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor)*(*bnd_T_zs)(x, y, time, this->F[d][i][j][k]);
							break;
						case BND_NEU:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							y = y0 + j * this->dy;
							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] =  dz / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_zs)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* dz / (*tmp)(this->Fstar[d][i][j][k])* (*bnd_T_zs)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor)*dz / (*tmp)(this->F[d][i][j][k])* (*bnd_T_zs)(x, y, time, this->F[d][i][j][k]);

							kk = k + 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;
							
							kk = k + 2;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							
							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else if (k == nz - 1) {
						switch (this->type_bnd_T_zf)
						{
						case BND_DIR:
							ii = i;
							jj = j;
							kk = k;
							IDCol = (ii*ny + jj)*nz + kk;
							x = x0 + i * this->dx;
							y = y0 + j * this->dy;

							accList.push_back(1.0);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] =  (*bnd_T_zf)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* (*bnd_T_zf)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor)*(*bnd_T_zf)(x, y, time , this->F[d][i][j][k]);
							break;
						case BND_NEU:
							
							ii = i;
							jj = j;
							kk = k - 2;
							
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(0.5);
							colList.push_back(IDCol);
							ID++;
							

							kk = k - 1;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(-2.0);
							colList.push_back(IDCol);
							ID++;

							kk = k;
							x = x0 + i * this->dx;
							y = y0 + j * this->dy;
							IDCol = (ii*ny + jj)*nz + kk;
							accList.push_back(1.5);
							colList.push_back(IDCol);
							ID++;
							b[IDRow] =  dz / (*tmp)(this->Fstar[d][i][j][k]) * (*bnd_T_zf)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] = CN_factor* dz / (*tmp)(this->Fstar[d][i][j][k]) * (*bnd_T_zf)(x, y, time + dt, this->Fstar[d][i][j][k]);
							//b[IDRow] += (1-CN_factor) * dz / (*tmp)(this->F[d][i][j][k]) * (*bnd_T_zf)(x, y, time, this->F[d][i][j][k]);

							break;
						case BND_CON:

							break;
						default:
							break;
						}
					}
					else {
						//*********************
						//i-1,j,k
						//********************
						ii = i - 1;
						jj = j;
						kk = k;
						IDCol = (ii*ny + jj)*nz + kk;


						//double alphax = 0.0;
						double alphax = 0.5 / dx * ((*tmp)(this->Fstar[d][i + 1][j][k]) - (*tmp)(this->Fstar[d][i - 1][j][k])) / (this->rho*this->cp);
						Fx = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						val = CN_factor * (alphax*dt*0.5 / dx - Fx);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;

						//double alphax_b = 0.0;
						double alphax_b = 0.5 / dx * ((*tmp)(this->F[d][i + 1][j][k]) - (*tmp)(this->F[d][i - 1][j][k])) / (this->rho*this->cp);
						Fx = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						val_b = (1.0 - CN_factor) * (alphax_b*dt*0.5 / dx - Fx)*this->F[d][ii][jj][kk];

						//*********************
						//i,j-1,k
						//*********************
						ii = i;
						jj = j - 1;
						kk = k;
						IDCol = (ii*ny + jj)*nz + kk;

						//
						//double alphay = 0.0;
						double alphay = 0.5 / dy * ((*tmp)(this->Fstar[d][i][j + 1][k]) - (*tmp)(this->Fstar[d][i][j - 1][k])) / (this->rho*this->cp);
						Fy = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						val = CN_factor * (alphay*dt*0.5 / dy - Fy);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;

						//double alphay_b = 0.0;
						double alphay_b = 0.5 / dy * ((*tmp)(this->F[d][i][j + 1][k]) - (*tmp)(this->F[d][i][j - 1][k])) / (this->rho*this->cp);
						Fy = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						val_b += (1.0 - CN_factor) * (alphay_b*dt*0.5 / dy - Fy)*this->F[d][ii][jj][kk];
						//********************
						//i,j,k-1
						//********************
						ii = i;
						jj = j;
						kk = k - 1;
						IDCol = (ii*ny + jj)*nz + kk;

						//double alphaz = 0.0;
						double alphaz = 0.5 / dz * ((*tmp)(this->Fstar[d][i][j][k + 1]) - (*tmp)(this->Fstar[d][i][j][k - 1])) / (this->rho*this->cp);
						Fz = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val = CN_factor * (alphaz*dt*0.5 / dz - Fz);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;

						//double alphaz_b = 0.0;
						double alphaz_b = 0.5 / dz * ((*tmp)(this->F[d][i][j][k + 1]) - (*tmp)(this->F[d][i][j][k - 1])) / (this->rho*this->cp);
						Fz = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val_b += (1.0 - CN_factor) * (alphaz_b*dt*0.5 / dz - Fz)*this->F[d][ii][jj][kk];
						//*********************
						//i,j,k
						//*********************
						ii = i;
						jj = j;
						kk = k;
						IDCol = (ii*ny + jj)*nz + kk;

						Fx = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						Fy = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						Fz = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val = 1+ CN_factor * ( 2 * Fx + 2 * Fy + 2 * Fz);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;

						Fx = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						Fy = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						Fz = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val_b += (1.0 - CN_factor) *( 2 * Fx + 2 * Fy + 2 * Fz)*this->F[d][ii][jj][kk];

						//********************
						//i,j,k+1
						//********************
						ii = i;
						jj = j;
						kk = k + 1;
						IDCol = (ii*ny + jj)*nz + kk;


						Fz = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val = CN_factor * (-alphaz * dt*0.5 / dz - Fz);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;

						Fz = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dz*dz);
						val_b += (1.0 - CN_factor) *(-alphaz_b * dt*0.5 / dz - Fz)*this->F[d][ii][jj][kk];
						//********************
						//i,j+1,k
						//********************
						ii = i;
						jj = j + 1;
						kk = k;
						IDCol = (ii*ny + jj)*nz + kk;


						Fy = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						val = CN_factor * (-alphay * dt*0.5 / dy - Fy);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;
						Fy = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dy*dy);
						val_b += (1.0 - CN_factor) *(-alphay_b * dt*0.5 / dy - Fy)*this->F[d][ii][jj][kk];
						//*******************
						//i +1,j,k
						//********************
						ii = i + 1;
						jj = j;
						kk = k;
						IDCol = (ii*ny + jj)*nz + kk;


						Fx = (*tmp)(this->Fstar[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						val = CN_factor * (-alphax * dt*0.5 / dx - Fx);
						accList.push_back(val);
						colList.push_back(IDCol);
						ID++;
						Fx = (*tmp)(this->F[d][ii][jj][kk]) / (this->rho*this->cp) * dt / (dx*dx);
						val_b += (1.0 - CN_factor) *(-alphax_b * dt*0.5 / dx - Fx)*this->F[d][ii][jj][kk];
						b[IDRow] = F[d][i][j][k] - val_b;
					}
				}
			}

		}
		int nbElem = accList.size();
		//rowList.push_back(ID);
		rowptr[nbEqs] = ID;






		double* acc = accList.data();
		int *colind = colList.data();
		//int *rowptr = rowList.data();


		double *acct = new double[nbElem];
		int *rowind = new int[nbElem];
		int *colptr = new int[nbEqs];

		SuperMatrix A;
		/* Convert the compressed row fromat to the compressed column format. */
		dCompRow_to_CompCol(nbEqs, nbEqs, nbElem, acc, colind, rowptr, &acct, &rowind, &colptr);
		dCreate_CompRow_Matrix(&A, nbEqs, nbEqs, nbElem, acct, rowind, colptr, SLU_NC, SLU_D, SLU_GE);

		int info;
		SuperMatrix L;
		SuperMatrix U;



		superlu_options_t options;
		int *perm_c = new int[nbEqs];
		int *perm_r = new int[nbEqs];
		SuperLUStat_t stat;


		SuperMatrix B, X;

		/*
		char equed[1];
		int            *etree;
		void           *work;
		int info, lwork, nrhs, ldx;

		double         *R, *C;
		double         *ferr, *berr;
		double u, rpg, rcond;

		yes_no_t       equil;
		trans_t trans;

		mem_usage_t mem_usage;
		*/

		//
		//  Create Super Right Hand Side.
		//
		dCreate_Dense_Matrix(&B, nbEqs, 1, b, nbEqs, SLU_DN, SLU_D, SLU_GE);
		

		set_default_options(&options);
		//options.ColPerm = NATURAL;
		//options.Trans = NOTRANS;
		//options.Equil = NO;
		//
		//  Initialize the statistics variables. 
		//

		 /* Defaults */
		/*
		lwork = 0;
		nrhs = 1;
		equil = YES;
		u = 1.0;
		trans = NOTRANS;
		*/
		/* Set the default values for options argument:
		options.Fact = DOFACT;
			options.Equil = YES;
			options.ColPerm = COLAMD;
		options.DiagPivotThresh = 1.0;
			options.Trans = NOTRANS;
			options.IterRefine = NOREFINE;
			options.SymmetricMode = NO;
			options.PivotGrowth = NO;
			options.ConditionNumber = NO;
			options.PrintStat = YES;
		*/
		
		
		set_default_options(&options);
		/*Can use command line input to modify the defaults. */
		
		//parse_command_line(argc, argv, &lwork, &u, &equil, &trans);
		/*options.Equil = equil;
		options.DiagPivotThresh = u;
		options.Trans = trans;
		*/
		StatInit(&stat);

		/*
		if (lwork > 0) {
			work = SUPERLU_MALLOC(lwork);
			if (!work) {
				ABORT("DLINSOLX: cannot allocate work[]");
			}
		}
		*/

		//
		//  Solve the linear system. 
		//
		dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
		
		/*dgssvx(&options, &A, perm_c, perm_r,etree, equed, R, C,
			&L, &U, work, lwork, &B, &X, &rpg, &rcond, ferr, berr,
			&mem_usage, &stat, &info);
			*/
		double *sol = (double*)((DNformat*)B.Store)->nzval;

		for (int i = 0; i < nx; i++) {
			for (int j = 0; j < ny; j++) {
				for (int k = 0; k < nz; k++) {
					Fstar[d][i][j][k] = sol[(i*ny + j)*nz + k];
				}
			}
		}

		/*******************************************/
		/*  Free memory.                           */
		/*******************************************/
		delete[] b;

		delete[] perm_c;
		delete[] perm_r;
		delete[] acct;
		delete[] rowind;
		delete[] colptr;

		accList.clear();
		colList.clear();
		delete[] rowptr;

		Destroy_SuperMatrix_Store(&A);
		Destroy_SuperMatrix_Store(&B);
		Destroy_SuperNode_Matrix(&L);
		Destroy_CompCol_Matrix(&U);

		StatFree(&stat);
	}
	
	
	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < ny; j++) {
			for (int k = 0; k < nz; k++) {
				F[d][i][j][k] = Fstar[d][i][j][k];
			}
		}
	}

	this->isFirstStep = true;
	this->time += dt;
	this->TimeVector.push_back(this->time);

}



void EAC3D::compute_RHS() {
	double oodx = 1. / this->dx;
	double oody = 1. / this->dy;
	double oodz = 1. / this->dz;


	double oodx2 = oodx * oodx;
	double oody2 = oody * oody;
	double oodz2 = oodz * oodz;

	func_ptr_dd tmp;

	if (this->isFirstStep) 
	{
		if (this->innerRK2Step == 0) {
			for (int d = 0; d < DIM_ARRAY; d++) {
				tmp = this->DCoef[d];
				for (int i = 1; i < nx - 1; i++) {
					for (int j = 1; j < ny - 1; j++) {
						for (int k = 1; k < nz - 1; k++) {
							
							this->RHS[d][i][j][k] = (*tmp)(this->F[d][i][j][k])* (
													  (this->F[d][i + 1][j][k] + this->F[d][i - 1][j][k] - 2.0*this->F[d][i][j][k]) *oodx2
													+ (this->F[d][i][j + 1][k] + this->F[d][i][j - 1][k] - 2.0*this->F[d][i][j][k]) *oody2
													+ (this->F[d][i][j][k + 1] + this->F[d][i][j][k - 1] - 2.0*this->F[d][i][j][k]) *oodz2
													);
							this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i + 1][j][k]) - (*tmp)(this->F[d][i - 1][j][k]))*0.25*oodx2*
													(this->F[d][i + 1][j][k] - this->F[d][i - 1][j][k]);
							this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i ][j+1][k]) - (*tmp)(this->F[d][i ][j-1][k]))*0.25*oody2*
													(this->F[d][i][j+1][k] - this->F[d][i][j-1][k]);
							this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i][j][k+1]) - (*tmp)(this->F[d][i][j][k-1]))*0.25*oodz2*
													(this->F[d][i][j][k+1] - this->F[d][i][j][k-1]);

						}
					}
				}
			}

		}
		if (this->innerRK2Step == 1) {
			for (int d = 0; d < DIM_ARRAY; d++) {
				tmp = this->DCoef[d];
				for (int i = 1; i < nx - 1; i++) {
					for (int j = 1; j < ny - 1; j++) {
						for (int k = 1; k < nz - 1; k++) {
							this->RHSnm1[d][i][j][k] = (*tmp)(this->Fstar[d][i][j][k])* 
								 ((this->Fstar[d][i + 1][j][k] + this->Fstar[d][i - 1][j][k] - 2.0*this->Fstar[d][i][j][k]) *oodx2
								+ (this->Fstar[d][i][j + 1][k] + this->Fstar[d][i][j - 1][k] - 2.0*this->Fstar[d][i][j][k]) *oody2
								+ (this->Fstar[d][i][j][k + 1] + this->Fstar[d][i][j][k - 1] - 2.0*this->Fstar[d][i][j][k]) *oodz2);
							this->RHSnm1[d][i][j][k] += ((*tmp)(this->Fstar[d][i + 1][j][k]) - (*tmp)(this->Fstar[d][i - 1][j][k]))*0.25*oodx2*
								(this->Fstar[d][i + 1][j][k] - this->Fstar[d][i - 1][j][k]);
							this->RHSnm1[d][i][j][k] += ((*tmp)(this->Fstar[d][i][j + 1][k]) - (*tmp)(this->Fstar[d][i][j - 1][k]))*0.25*oody2*
								(this->Fstar[d][i][j + 1][k] - this->Fstar[d][i][j - 1][k]);
							this->RHSnm1[d][i][j][k] += ((*tmp)(this->Fstar[d][i][j][k + 1]) - (*tmp)(this->Fstar[d][i][j][k - 1]))*0.25*oodz2*
								(this->Fstar[d][i][j][k + 1] - this->Fstar[d][i][j][k - 1]);
						}
					}
				}
			}
		}
	}
	else {
		for (int d = 0; d < DIM_ARRAY; d++) {
			tmp = this->DCoef[d];
			for (int i = 1; i < nx - 1; i++) {
				for (int j = 1; j < ny - 1; j++) {
					for (int k = 1; k < nz - 1; k++) {
						this->RHSnm1[d][i][j][k] = this->RHS[d][i][j][k];
						
						//grad^2 F
						this->RHS[d][i][j][k] = (*tmp)(this->F[d][i][j][k])*
												 ((this->F[d][i + 1][j][k] + this->F[d][i - 1][j][k] - 2.0*this->F[d][i][j][k]) *oodx2
												+ (this->F[d][i][j + 1][k] + this->F[d][i][j - 1][k] - 2.0*this->F[d][i][j][k]) *oody2
												+ (this->F[d][i][j][k + 1] + this->F[d][i][j][k - 1] - 2.0*this->F[d][i][j][k]) *oodz2);

						this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i + 1][j][k]) - (*tmp)(this->F[d][i - 1][j][k]))*0.25*oodx2*
							(this->F[d][i + 1][j][k] - this->F[d][i - 1][j][k]);
						this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i][j + 1][k]) - (*tmp)(this->F[d][i][j - 1][k]))*0.25*oody2*
							(this->F[d][i][j + 1][k] - this->F[d][i][j - 1][k]);
						this->RHS[d][i][j][k] += ((*tmp)(this->F[d][i][j][k + 1]) - (*tmp)(this->F[d][i][j][k - 1]))*0.25*oodz2*
							(this->F[d][i][j][k + 1] - this->F[d][i][j][k - 1]);

						//grad D . grad F
						/*this->RHS[d][i][j][k] =   (this->F[d][i+1][j][k] -this->F[d][i-1][j][k] ) *0.5*oodx
											+ (this->F[d][i][j+1][k] -this->F[d][i][j-1][k] ) *0.5*oody
											+ (this->F[d][i][j][k+1] -this->F[d][i][j][k-1] ) *0.5*oodz;*/
					}
				}
			}
		}

	}

	
}

void EAC3D::timeMarching(bool onlyRK2) {
	
	if (this->isFirstStep || onlyRK2) {
		this->innerRK2Step = 0;
		
		this->set_BNDXS(bnd_T_xs, TID, type_bnd_T_xs);
		this->set_BNDXF(bnd_T_xf, TID, type_bnd_T_xf);
		this->set_BNDYS(bnd_T_ys, TID, type_bnd_T_ys);
		this->set_BNDYF(bnd_T_yf, TID, type_bnd_T_yf);
		this->set_BNDZS(bnd_T_zs, TID, type_bnd_T_zs);
		this->set_BNDZF(bnd_T_zf, TID, type_bnd_T_zf);

		if (this->DIM_ARRAY >= 2) {
			this->set_BNDXS(bnd_H_xs, HID, type_bnd_H_xs);
			this->set_BNDXF(bnd_H_xf, HID, type_bnd_H_xf);
			this->set_BNDYS(bnd_H_ys, HID, type_bnd_H_ys);
			this->set_BNDYF(bnd_H_yf, HID, type_bnd_H_yf);
			this->set_BNDZS(bnd_H_zs, HID, type_bnd_H_zs);
			this->set_BNDZF(bnd_H_zf, HID, type_bnd_H_zf);
		}
		this->compute_RHS();


		for (int d = 0; d < DIM_ARRAY; d++) {
			for (int i = 1; i < nx - 1; i++) {
				for (int j = 1; j < ny - 1; j++) {
					for (int k = 1; k < nz - 1; k++) {	
						this->Fstar[d][i][j][k] = this->F[d][i][j][k] +  0.5*this->dt*this->RHS[d][i][j][k];
					}
				}
			}
		}
		this->time += 0.5*dt;

		this->innerRK2Step = 1;

		this->set_BNDXS(bnd_T_xs, TID, type_bnd_T_xs);
		this->set_BNDXF(bnd_T_xf, TID, type_bnd_T_xf);
		this->set_BNDYS(bnd_T_ys, TID, type_bnd_T_ys);
		this->set_BNDYF(bnd_T_yf, TID, type_bnd_T_yf);
		this->set_BNDZS(bnd_T_zs, TID, type_bnd_T_zs);
		this->set_BNDZF(bnd_T_zf, TID, type_bnd_T_zf);

		if (this->DIM_ARRAY >= 2) {
			this->set_BNDXS(bnd_H_xs, HID, type_bnd_H_xs);
			this->set_BNDXF(bnd_H_xf, HID, type_bnd_H_xf);
			this->set_BNDYS(bnd_H_ys, HID, type_bnd_H_ys);
			this->set_BNDYF(bnd_H_yf, HID, type_bnd_H_yf);
			this->set_BNDZS(bnd_H_zs, HID, type_bnd_H_zs);
			this->set_BNDZF(bnd_H_zf, HID, type_bnd_H_zf);
		}

		this->compute_RHS();

		for (int d = 0; d < DIM_ARRAY; d++) {
			for (int i = 1; i < nx - 1; i++) {
				for (int j = 1; j < ny - 1; j++) {
					for (int k = 1; k < nz - 1; k++) {
						this->F[d][i][j][k] += this->dt*this->RHSnm1[d][i][j][k];
					}
				}
			}
		}
		
		this->time += 0.5*dt;
		
		this->isFirstStep = false;
	}
	else {
		this->innerRK2Step = 0;

		this->set_BNDXS(bnd_T_xs, TID, type_bnd_T_xs);
		this->set_BNDXF(bnd_T_xf, TID, type_bnd_T_xf);
		this->set_BNDYS(bnd_T_ys, TID, type_bnd_T_ys);
		this->set_BNDYF(bnd_T_yf, TID, type_bnd_T_yf);
		this->set_BNDZS(bnd_T_zs, TID, type_bnd_T_zs);
		this->set_BNDZF(bnd_T_zf, TID, type_bnd_T_zf);
		if (this->DIM_ARRAY >= 2) {
			this->set_BNDXS(bnd_H_xs, HID, type_bnd_H_xs);
			this->set_BNDXF(bnd_H_xf, HID, type_bnd_H_xf);
			this->set_BNDYS(bnd_H_ys, HID, type_bnd_H_ys);
			this->set_BNDYF(bnd_H_yf, HID, type_bnd_H_yf);
			this->set_BNDZS(bnd_H_zs, HID, type_bnd_H_zs);
			this->set_BNDZF(bnd_H_zf, HID, type_bnd_H_zf);
		}
		this->compute_RHS();

		for (int d = 0; d < DIM_ARRAY; d++) {
			for (int i = 1; i < nx - 1; i++) {
				for (int j = 1; j < ny - 1; j++) {
					for (int k = 1; k < nz - 1; k++) {
						this->F[d][i][j][k] += 0.5*this->dt*(3.*this->RHS[d][i][j][k] - this->RHSnm1[d][i][j][k]);
					}
				}
			}
		}
		this->time += dt;
	}
	this->TimeVector.push_back(this->time);
}

void EAC3D::set_T_lambda(double(*func)(double)) {
	this->DCoef[TID] = func;
}

void EAC3D::set_H_lambda(double(*func)(double)) {
	this->DCoef[HID] = func;
}

void EAC3D::set_rho(double val) {
	this->rho = val;
}

void EAC3D::set_cp(double val) {
	this->cp = val;
}

void EAC3D::set_BNDXS(double(*func)(double, double, double, double), int id, int type) {
	int i = 0;

	double y, z;
	switch (type)
	{
	case BND_DIR:
		for (int j = 0; j < ny ; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = (*func)(y, z, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] = (*func)(y, z, this->time, this->Fstar[id][i][j][k]);
				}
			}
		}
		break;
	case BND_NEU:
		for (int j = 0; j < ny ; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1./3.*(4*this->F[id][i + 1][j][k] -  this->F[id][i + 2][j][k] + 2* this->dx*(*func) (y, z, this->time, this->F[id][i][j][k])/(*DCoef[id])( this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1./3.*(4 * this->Fstar[id][i + 1][j][k] - this->Fstar[id][i + 2][j][k] + 2 * this->dx*(*func) (y, z, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}

void EAC3D::set_BNDXF(double(*func)(double, double, double, double), int id, int type) {
	int i = this->nx - 1;
	double y, z;
	switch (type) {
	case BND_DIR:
		for (int j = 0; j < ny ; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = (*func)(y, z, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] = (*func)(y, z, this->time, this->Fstar[id][i][j][k]) ;
				}
			}
		}
		break;
	case BND_NEU:
		for (int j = 0; j < ny ; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1. / 3.*(4 * this->F[id][i - 1][j][k] - this->F[id][i - 2][j][k] + 2 * this->dx*(*func) (y, z, this->time, this->F[id][i][j][k]) / (*DCoef[id])(this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1. / 3.*(4 * this->Fstar[id][i - 1][j][k] - this->Fstar[id][i - 2][j][k] + 2 * this->dx*(*func) (y, z, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}


//BND_Y

void EAC3D::set_BNDYS(double(*func)(double, double, double, double), int id, int type) {
	int j = 0;
	double x, z;
	switch (type) {
	case BND_DIR:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = (*func)(x, z, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] =(*func)(x, z, this->time, this->Fstar[id][i][j][k]);
				}
				
			}
		}
		break;
	case BND_NEU:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1. / 3.*(4 * this->F[id][i ][j+1][k] - this->F[id][i ][j+2][k] + 2 * this->dy*(*func) (x, z, this->time, this->F[id][i][j][k]) / (*DCoef[id])(this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1. / 3.*(4 * this->Fstar[id][i][j + 1][k] - this->Fstar[id][i][j + 2][k] + 2 * this->dy*(*func) (x, z, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}

void EAC3D::set_BNDYF(double(*func)(double, double, double, double), int id, int type) {
	int j = this->ny - 1;

	double x, z;
	switch (type) {
	case BND_DIR:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = (*func)(x, z, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] = (*func)(x, z, this->time, this->Fstar[id][i][j][k]) ;
				}
			}
		}
		break;
	case BND_NEU:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int k = 0; k < nz ; k++) {
				z = z0 + k * this->dz;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1. / 3.*(4 * this->F[id][i][j - 1][k] - this->F[id][i][j - 2][k] + 2 * this->dy*(*func) (x, z, this->time, this->F[id][i][j][k]) / (*DCoef[id])(this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1. / 3.*(4 * this->Fstar[id][i][j - 1][k] - this->Fstar[id][i][j - 2][k] + 2 * this->dy*(*func) (x, z, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}

//BND Z

void EAC3D::set_BNDZS(double(*func)(double, double, double, double), int id, int type) {
	int k = 0;
	double x, y;
	switch (type) {
	case BND_DIR:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int j = 0; j < ny ; j++) {
				y = y0 + j * this->dy;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = (*func)(x, y, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] = (*func)(x, y, this->time, this->Fstar[id][i][j][k]);
				}
				
			}
		}
		break;
	case BND_NEU:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int j = 0; j < ny ; j++) {
				y = y0 + j * this->dy;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1. / 3.*(4 * this->F[id][i][j][k+1] - this->F[id][i][j][k+2] +2 * this->dz*(*func) (x, y, this->time, this->F[id][i][j][k]) / (*DCoef[id])(this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1. / 3.*(4 * this->Fstar[id][i][j][k + 1] - this->Fstar[id][i][j][k + 2] + 2 * this->dz*(*func) (x, y, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
				
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}

void EAC3D::set_BNDZF(double(*func)(double, double, double, double), int id, int type) {
	int k = this->nz - 1;
	double x, y;
	switch (type) {
	case BND_DIR:
		for (int i = 0; i < nx; i++) {
			x = x0 + i * this->dx;
			for (int j = 0; j < ny ; j++) {
				y = y0 + j * this->dy;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] =  (*func)(x, y, this->time, this->F[id][i][j][k]);
				}
				else {
					this->Fstar[id][i][j][k] = (*func)(x, y, this->time, this->Fstar[id][i][j][k]) ;
				}
				
			}
		}
		break;
	case BND_NEU:
		for (int i = 0; i < nx ; i++) {
			x = x0 + i * this->dx;
			for (int j = 0; j < ny ; j++) {
				y = y0 + j * this->dy;
				if (this->innerRK2Step == 0) {
					this->F[id][i][j][k] = 1. / 3.*(4 * this->F[id][i][j][k - 1] - this->F[id][i][j][k -2] + 2 * this->dz*(*func) (x, y, this->time, this->F[id][i][j][k]) / (*DCoef[id])(this->F[id][i][j][k]));
				}
				else {
					this->Fstar[id][i][j][k] = 1. / 3.*(4 * this->Fstar[id][i][j][k -1] - this->Fstar[id][i][j][k - 2] + 2 * this->dz*(*func) (x, y, this->time, this->Fstar[id][i][j][k]) / (*DCoef[id])(this->Fstar[id][i][j][k]));
				}
				
			}
		}
		break;
	case BND_CON:

		break;
	default:
		break;
	}
}

void EAC3D::set_T_BNDXS(double(*bnd)(double, double, double, double), int type) {
	bnd_T_xs = bnd;	
	this->type_bnd_T_xs = type;
}
void EAC3D::set_T_BNDXF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_T_xf = bnd;
	this->type_bnd_T_xf = type;
}
void EAC3D::set_T_BNDYS(double(*bnd)(double, double, double, double), int type) {
	this->bnd_T_ys = bnd;
	this->type_bnd_T_ys = type;
}
void EAC3D::set_T_BNDYF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_T_yf = bnd;
	this->type_bnd_T_yf = type;
}
void EAC3D::set_T_BNDZS(double(*bnd)(double, double, double, double), int type) {
	this->bnd_T_zs = bnd;
	this->type_bnd_T_zs = type;
}
void EAC3D::set_T_BNDZF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_T_zf = bnd;
	this->type_bnd_T_zf = type;
}

void EAC3D::set_H_BNDXS(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_xs = bnd;
	this->type_bnd_H_xs = type;
}
void EAC3D::set_H_BNDXF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_xf = bnd;
	this->type_bnd_H_xf = type;
}

void EAC3D::set_DStrainODH(double(*func)(double)){
	this->DStrainODH = func;
}
void EAC3D::set_DStrainODT(double(*func)(double)) {
	this->DStrainODT = func;
}

void EAC3D::set_Restrain(double(*func_restrain)(double, double, double)) {
	this->Restrain_func = func_restrain;
}
void EAC3D::set_Relaxation(double(*func_J_relax)(double, double)) {
	this->J_relax = func_J_relax;
}

void EAC3D::set_H_BNDYS(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_ys = bnd;
	this->type_bnd_H_ys = type;
}
void EAC3D::set_H_BNDYF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_yf = bnd;
	this->type_bnd_H_yf = type;
}
void EAC3D::set_H_BNDZS(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_zs = bnd;
	this->type_bnd_H_zs = type;
}
void EAC3D::set_H_BNDZF(double(*bnd)(double, double, double, double), int type) {
	this->bnd_H_zf = bnd;
	this->type_bnd_H_zf = type;
}


void EAC3D::init_field(double(*init)(double, double, double), int id) {
	double x, y, z;

	for (int i = 0; i < nx; i++) {
		x = x0 + i * this->dx;
		for (int j = 0; j < ny; j++) {
			y = y0 + j * this->dy;
			for (int k = 0; k < nz; k++) {
				z = z0 + k * this->dz;
				this->F[id][i][j][k] = (*init)(x, y, z);
				this->Finit[id][i][j][k] = this->F[id][i][j][k];
			}
		}
	}

	
}

