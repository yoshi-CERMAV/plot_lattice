//
//  find_orientation.cpp
//  
//
//  Created by Yoshiharu Nishiyama on 29/11/2023.
//

//#include "find_orientation.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <gsl/gsl_multimin.h>
#include <rotator.h>
#include "diffraction_data.h"
using namespace std;
//x = sin(phi)cos(theta)
//y = sin(phi)sin(theta)
//z = cos(phi)

FILE *gp;
int number_of_clusters;
int number_of_data;
double threshold;
int *histogram;
double *log_list;

float *xyz;
float *projection;
float step;
float **cluster;
int cluster_count;
const double rad= M_PI/180;
gsl_vector *ss, *x;

ofstream fo;

float candidate_axe[3];
static float axe0[3], axe1[3];

rotator_axe<float> r0, r1;

void copy3(float *a, float *b){
    a[0] = b[0];
    a[1] = b[1];
    a[2] = b[2];
}
void cross_product(const float a[3], const float b[3], float *c)
{
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

void set_candidate(const float axe[3])
{
    for(int i = 0; i < 3; i++) candidate_axe[i] = axe[i];
    float rot_axe[3] = {-candidate_axe[1], candidate_axe[0], 0};
    r0.set_pole(rot_axe);
    float rot_axe1[3];
    cross_product(rot_axe,candidate_axe, rot_axe1);
    r1.set_pole(rot_axe1);
}

double calc_penalty(float *projection, int number_of_data);

struct Entropy{
    double phi;
    double theta;
    double value;
    double x[3];
    void init(){
        double st = sin(theta);
        x[0] = st * cos(phi);
        x[1] = st * sin(phi);
        x[2] = cos(theta);
    }
    int print (){cout << phi<< " "<<theta<<" "<<value<<endl; return 0;}
    double angle(Entropy &e){
        return x[0] * e.x[0]
              +x[1] * e.x[1]
              +x[2] * e.x[2];
    }
};

float dot(float x[], float y[])
{
    return  x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}
void angle2vector(float &theta, float &phi, float x[])
{
    double st = sin(theta);
    x[0] = st * cos(phi);
    x[1] = st * sin(phi);
    x[2] = cos(theta);
//    fo << x[0] <<" "<< x[1]<<" "<<x[2]<<" ";
}

int calc_projection(float *data, float *projection, int number_of_data, float x[3])
{
    float *ptr = data;
    for(int i = 0; i < number_of_data; i++, ptr+=3){
        projection[i] = x[0] * ptr[0] + x[1] * ptr[1] + x[2]*ptr[2];
    }
    return 0;
}

float sqr(float &x){return x*x;}
int average(float *x0, float *x1, float &av, float &stdev)
{
    float sum=0;
    float sum2=0;
    int n = x1 - x0;

    while(x0 != x1){
        sum+= *x0;
        sum2+= sqr(*x0);
        x0++;
    }
    av = sum/n;
    stdev = sum2/n - sqr(av);
//    cout <<" "<< av <<" "<<stdev<<" "<<n<<endl;
    return 0;
}




double
my_f (const gsl_vector *v, void *params)
{
    r0.rotate(gsl_vector_get(v, 0));
    r1.rotate(gsl_vector_get(v, 1));
    r0.combine(r1);
    float axe[3];
    for(int i = 0; i < 3; i++) axe[i] = candidate_axe[i];
    r0.apply(axe);
    calc_projection(xyz, projection, number_of_data, axe);
    return calc_penalty(projection, number_of_data);
}
int minimize(const float axe[3]){
    
    size_t iter = 0;
    int status;
    double size;
    set_candidate(axe);
    cout << "candidates OK"<<endl;
    x  = gsl_vector_alloc(2);
    ss = gsl_vector_alloc(2);
    cout << "vectors allocated"<<endl;
    gsl_vector_set_all (x, 0);
    gsl_vector_set_all (ss, 0.01);
    gsl_multimin_function func;
    func.n = 2;
    func.f = my_f;
    const gsl_multimin_fminimizer_type *T =
      gsl_multimin_fminimizer_nmsimplex2;
    gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc (T, 2);
    gsl_multimin_fminimizer_set (s, &func, x, ss);
    do
    {
        iter++;
        status = gsl_multimin_fminimizer_iterate(s);
        
        if (status)
            break;
        
        size = gsl_multimin_fminimizer_size (s);
        status = gsl_multimin_test_size (size, 1e-6);
        
        if (status == GSL_SUCCESS)
        {
            printf ("converged to minimum at\n");
        }
        
        printf ("%5d %10.3e %10.3e f() = %7.3f size = %.3f\n",
                iter,
                gsl_vector_get (s->x, 0),
                gsl_vector_get (s->x, 1),
                s->fval, size);
    }    while (status == GSL_CONTINUE && iter < 100);
    r0.rotate(gsl_vector_get(s->x, 0));
    r1.rotate(gsl_vector_get(s->x, 0));
    r0.combine(r1);
    r0.apply(candidate_axe);
    return status;
}


float norm(float *xyz)
{
   float sum = 0;
   for(int i = 0; i < 3; i++) sum+= sqr(xyz[i]);
   return sqrt(sum);
}

float get_max(float *xyz, int n)
{
   float max = 0;
   float *ptr = xyz;
   for(int i = 0; i < n; i++, ptr+=3){
      float temp = norm(ptr);
      if(max < temp) max = temp;
   }
   return max;
}

float calc_block(float **ptr, float *end, float max){
   double sum = 0;
   double sum2 = 0;
   int count = 0;
   while(**ptr < max &&  *ptr != end){
      sum+= **ptr;
      sum2 += sqr(**ptr);
      count++;
      (*ptr)++;
   }
   if(!count) return 0;
   else if(count==1) return .3;
   else{
      sum/=count;
      sum2/=count;
       if(sum2 < sum*sum){
            return 0;
       }
      return sqrt((sum2-sum*sum)/count);
   }
}
#ifdef BACK

double calc_penalty(float *projection, int number_of_data)
{
    sort(projection, projection+number_of_data);
    float range = projection[number_of_data-1]- projection[0];
 //   step = range/number_of_data;
   float rstep = number_of_data/range;
   float shift  = number_of_data/2;
   for(int i = 0; i < number_of_data; i++) projection[i] *= rstep;
   for(int i = 0; i < number_of_data; i++) projection[i] += shift;
   float *ptr = projection;
   float *data_end = ptr+number_of_data;
   float max = 1.;
   double penalty = 0;
   for(int i = 0; i < number_of_data; i++){
      double temp =  calc_block(&ptr, data_end, max+=1.);
      penalty += temp;
   }
   return penalty;
}
#endif

    
double calc_penalty(float *projection, int number_of_data)
{
   float rstep = 1./step;
   float shift  = number_of_data/2;
   for(int i = 0; i < number_of_data; i++) projection[i] *= rstep;
   for(int i = 0; i < number_of_data; i++) projection[i] += shift;
   sort(projection, projection+number_of_data);
   float *ptr = projection;
   float *data_end = ptr+number_of_data;
   float max = 1.;
   double penalty = 0;
   for(int i = 0; i < number_of_data; i++){
      double temp =  calc_block(&ptr, data_end, max+=1.);
      penalty += temp;
   }
   return penalty;
}

float dot_product (const float *x, const float *y){
   return x[0] * y[0] +
          x[1] * y[1] +
          x[2] * y[2];
}
int search_orthogonal(const float *tpe, const float *ax, int number_of_points, int &pos )
{
  cout <<"number of points "<< number_of_points<<endl;
   const float *x= tpe;
   float min =1000000;
   int mini =0;
   for(int i = 0; i < number_of_points; i++, x+=4){
      float dot = dot_product (x, ax);
      if(fabs(dot) > 0.5) continue;
      if(x[3] < min) {min = x[3]; mini = i;}
   }
   pos = mini*4;
   cout << mini<<" tpe " <<tpe[pos] <<" "<< tpe[pos+1] <<" "<<tpe[pos+2] <<" "<< tpe[pos+3]<<endl;
}
#ifdef AAA
int read_file(const char filename[], float *xyz)
{
    struct stat st;
    if(stat(filename,&st) != 0) {
        cerr<<"file does not exist "<<filename<<endl;
        exit(0);
    }
    diffraction_data dat;
    number_of_data = st.st_size / sizeof(dat);
    cout << number_of_data<<endl;
    diffraction_data *diff_dat= new diffraction_data[number_of_data];
    xyz = new float[number_of_data*3];
    fi.read(reinterpret_cast<char *>(diff_dat), sizeof(diff_dat[0])*number_of_data);
    float *ptr = xyz;
    for(int i = 0; i < number_of_data; i++, ptr+=3){
        ptr[0] = diff_dat[i].x();
        ptr[0] = diff_dat[i].y();
    }
    return number_of_data;
}
#endif

int refine(float *in_xyz, int number_of_points, float *angles)
{
    float max = get_max(xyz, number_of_points);
    step = max*2./number_of_points;
    projection = new float[number_of_points];
    minimize(axe0);
    minimize(axe1);
    rotator<float > rot;
    rot.bring_on_xy(axe0, axe1, angles);

}


int get_axes(float *in_xyz, int number_of_points, float angle_step, float *angles)
{
    xyz = in_xyz;
    float max;
    number_of_data =number_of_points;
    max = get_max(xyz, number_of_points);
    cout <<"max = "<< max<<endl;
    step = max*2./number_of_points;
    
    projection = new float[number_of_points];
    cout <<"allocated projection"<<endl;
    float phi = 0;

//    float angle_step = 0.01;
    float theta = angle_step*0.3;
    float r2pi = 0.5/M_PI;
    float d_theta = sqr(angle_step) * r2pi;
    float *tpe = new float[256*256*4];
    int count = 0;
    float min_e = 100000;
    int min_ei = 0;
    fo.open("ene");
    float *ptr = tpe;
    float *ptr_xyz = xyz;
//    for(int i = 0; i < 10; i++, ptr_xyz+=3){
//        cout << ptr_xyz[0] << " "<<ptr_xyz[1]<<" "<<ptr_xyz[2]<<endl;
//    }
 //   float M_PI_2 = M_PI/2;
    while(theta < M_PI_2){
        angle2vector(theta, phi, ptr);
        double st = 1/sin(theta);
        theta += d_theta*st;
        phi += angle_step *st;
        ptr+= 4;
        count ++;
    }
    ptr = tpe;
    for(int i = 0; i < count; i++, ptr+=4){
 //   for(int i = 0; i < 1; i++, ptr+=4){
        calc_projection(xyz, projection, number_of_points, ptr);
        double ene = calc_penalty(projection, number_of_points);
        ptr[3] = ene;
        if(min_e > ene) {min_e = ene; min_ei = i;}
   }
   cout << tpe[min_ei*4] <<" "<<tpe[min_ei*4+1]<<" "<<tpe[min_ei*4+2]<<" "<<tpe[min_ei*4+3]<<endl;;
   int pos;
   search_orthogonal(tpe, tpe+min_ei*4, count, pos);
    ofstream fo_tpe("tpe");
    fo_tpe.write(reinterpret_cast<char *>(tpe), sizeof(float)*4*count);
    cout << "tpe count "<<count<<endl;
    minimize(tpe+min_ei*4);
    copy3(axe0, candidate_axe);
    minimize(tpe+pos);
    copy3(axe1, candidate_axe);
    rotator<float > rot;
    rot.bring_on_xy(axe0, axe1, angles);
 //   float angles[3];
//    float *pt= tpe+min_ei*4;
//    float *pt1= tpe+pos;

//    cout <<"axe 0 "<<pt[0] <<" "<<pt[1] <<" "<< pt[2]<<endl;
//    cout <<"axe 1 "<<pt1[0] <<" "<<pt1[1] <<" "<< pt1[2]<<endl;

 //  rot.bring_on_xy(tpe+min_ei*4, tpe+pos, angles);

   ptr = xyz;
//    rot.dump();
 //   fo << number_of_points <<endl;
   for(int i = 0; i < number_of_points; i++, ptr +=3){
     rot.apply(ptr);
     fo << ptr[0] << " "<< ptr[1] << " "<< ptr[2]<<endl;
   }
    fo.close();
    delete [] projection;
    delete [] tpe;
}
