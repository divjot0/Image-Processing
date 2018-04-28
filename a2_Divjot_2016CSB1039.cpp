#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace cv;

Point2f Tra(int x, int y, vector<Point2f> vi, vector<Point2f> vj)
{
	Point2f a = Point2f(x,y);
	int red=0;
	int i;

	vector<Point2f>::iterator t = vj.begin();
	for( vector<Point2f>::iterator it = vi.begin(); it != vi.end(); it++)
    {
        if ((*it).x==y && (*it).y==x)
        {
        	red=1;
        	break;
        }
        t++;
//      cout << "het" << endl;
    }

    if (red==1)
	{
		a = Point2f((*t).y,(*t).x);
	}

//	cout << "given was " << x << " " << y <<  " returning " << a.x << " " << a.y << endl;

	return a;
}

bool is_inside(float x, float y, int x1, int y1, int x2, int y2, int x3, int y3)
{
	float mean_x = (float)(x1+x2+x3)/3.0;
	float mean_y = (float)(y1+y2+y3)/3.0;
	float X,Y,C;
	//line representation Yy + Xx = C;
	//x1 and x2 line
	Y = x2-x1;
	X = y1-y2;
	C = y1*(x2-x1) - x1*(y2-y1);
	if ((Y*mean_y + X*mean_x - C)*(Y*y + X*x - C)<0)
	{
		return 0;
	}

	//x2,x3 line
	Y = x3-x2;
	X = y2-y3;
	C = y2*(x3-x2) - x2*(y3-y2);
	if ((Y*mean_y + X*mean_x - C)*(Y*y + X*x - C)<0)
	{
		return 0;
	}

	//x3,x1 line
	Y = x1-x3;
	X = y3-y1;
	C = y3*(x1-x3) - x3*(y1-y3);
	if ((Y*mean_y + X*mean_x - C)*(Y*y + X*x - C)<0)
	{
		return 0;
	}
	return 1;
}

int Median(int i, int j, Mat New, int flag)
{
	int arr[25];
	int f=0;
	for (int k = -2; k < 3; ++k)
	{
		for (int r = -2; r < 3; ++r)
		{
			arr[f] = New.at<Vec3b>(i+k,j+r)[flag];
			f++;
		}
	}
	sort(arr,arr+25);
	if (New.at<Vec3b>(i,j)[flag])
	{
		return New.at<Vec3b>(i,j)[flag];
	}
	return arr[12];
}

int main(int argc, char const *argv[])
{
	bool find =1;
	float M[6];
	float V[6];
	Mat I,J;
	cout << "Press 0 for affine change and 1 for the morphing of one image to another:\t";
	cin >> find;

	int n;
	vector<Point2f> vi;
	vector<Point2f> vj;

	if (find)
	{
		string first;
		cout << "Enter the path of the image to be transformed:\t";
		cin >> first;
		I = imread(first, CV_LOAD_IMAGE_COLOR);
		if (!I.data)
		{
			cout << "Image failed to load!" << endl;
			return 1;
		}
		string second;
		cout << "Enter the path of the image to be transformed into:\t";
		cin >> second;
		J = imread(second, CV_LOAD_IMAGE_COLOR);
		if (!J.data)
		{
			cout << "Image failed to load!" << endl;
			return 1;
		}

		cout << "Enter number of tie points:\t";
		cin >> n;
		string file1, file2;

		cout << "Enter first images' tie points:\t";
//		cin >> file1;
//		ifstream ss;
//		ss.open(file1.c_str()); 
		for (int i = 0; i < n; ++i)
		{
			int a, b;
			cin >> a >> b;
			Point2f p(a, b);
			vi.push_back(p);
		}
		cout << "Enter second images' tie points:\t";
//		cin >> file2;
//		ifstream r;
//		r.open(file2.c_str());
		for (int i = 0; i < n; ++i)
		{
			int a, b;
			cin >> a >> b;
			Point2f p(a, b);
			vj.push_back(p);
		}
	}
	else
	{
		string first;
		cout << "Enter the path of the image:\t";
		cin >> first;
		I = imread(first, CV_LOAD_IMAGE_COLOR);
		if (!I.data)
		{
			cout << "Image failed to load!" << endl;
			return 1;
		}
		cout << "Enter the 2x3 matrix:\t";
		for (int i = 0; i < 6; ++i)
		{
			cin >> V[i];
		}
		n=3;
		vi.push_back(Point2f(0,0));
		vi.push_back(Point2f(0,1));
		vi.push_back(Point2f(1,0));
		vj.push_back(Point2f(V[2],V[5]));
		vj.push_back(Point2f(V[1]+V[2],V[4]+V[5]));
		vj.push_back(Point2f(V[0]+V[2],V[3]+V[5]));
	}

	string red1;
	cout << "Enter the path of the directory to store the images:\t";
	cin >> red1;

	int transitions;
	cout << "Enter number of transitions:\t";
	cin >> transitions;

	float times = 1/(float)transitions;

	Size size = I.size();
	Rect R(0, 0, size.width, size.height);
	Subdiv2D a(R);
	//a.initDelaunay(R);

	for( vector<Point2f>::iterator it = vi.begin(); it != vi.end(); it++)
    {
        a.insert(*it);
    }

	vector<Vec6f> triangle;
	a.getTriangleList(triangle);

	cout << "These are the triangles:\n";
	for (int i = 0; i < triangle.size(); ++i)
	{
		cout << triangle[i][1]<< " " << triangle[i][0] << " " << triangle[i][3] << " " << triangle[i][2]<< " " << triangle[i][5] << " " << triangle[i][4] << endl;
	}
	
	vector<vector<Point2f> > checks;

	for (int cur_t=0; cur_t<triangle.size(); cur_t++)
	{
		vector<Point2f> v;	
		for (int i = 0; i < I.rows; ++i)
		{
			for (int j = 0; j < I.cols; ++j)
			{
				if (is_inside(i, j, triangle[cur_t][1], triangle[cur_t][0], triangle[cur_t][3], triangle[cur_t][2], triangle[cur_t][5], triangle[cur_t][4]))
				{
					Point2f a(i,j);
					v.push_back(a);
				}
			}
		}
		checks.push_back(v);
	}
	int te=97; int tee = 97;
	for (float alpha =0; alpha<=1; alpha=times+alpha)
	{
		Mat New = Mat::zeros(I.rows, I.cols, I.type());
//		Mat e = Mat::zeros(I.rows, I.cols, I.type());
		Mat New2 = Mat::zeros(I.rows, I.cols, I.type());
		vector<vector<Point2f> >::iterator iter = checks.begin();
		if (find)
		{
			for (int cur_t = 0; cur_t < triangle.size(); ++cur_t)
			{
				Point2f srcTri[3];
				Point2f desTri[3];
				Point2f howTri[3];
				srcTri[0] = Point2f(triangle[cur_t][1], triangle[cur_t][0]);
				srcTri[1] = Point2f(triangle[cur_t][3], triangle[cur_t][2]);
				srcTri[2] = Point2f(triangle[cur_t][5], triangle[cur_t][4]);
				desTri[0] = Tra(triangle[cur_t][1], triangle[cur_t][0], vi, vj);
				desTri[1] = Tra(triangle[cur_t][3], triangle[cur_t][2], vi, vj);
				desTri[2] = Tra(triangle[cur_t][5], triangle[cur_t][4], vi, vj);
				howTri[0] = Point2f((((1-alpha)*(float)srcTri[0].x + (float)(desTri[0].x)*alpha)),((1-alpha)*(float)srcTri[0].y + (float)(desTri[0].y)*alpha));
				howTri[1] = Point2f((((1-alpha)*(float)srcTri[1].x + (float)(desTri[1].x)*alpha)),((1-alpha)*(float)srcTri[1].y + (float)(desTri[1].y)*alpha));
				howTri[2] = Point2f((((1-alpha)*(float)srcTri[2].x + (float)(desTri[2].x)*alpha)),((1-alpha)*(float)srcTri[2].y + (float)(desTri[2].y)*alpha));

		//		cout << " srcTri[0] is " << desTri[0].x << " " << desTri[0].y << endl;		
		//		cout << " desTri[0] is " << srcTri[0].x << " " << srcTri[0].y << endl;
		//		cout << " howTri[0] is " << howTri[0].x << " " << howTri[0].y << endl;
		//		cout << I.rows << " sss jjjjjjjjjjjjjjj " << I.cols << endl;

				float x1,x2,x3,y1,y2,y3,xx1,xx2,xx3,yy1,yy2,yy3;
				x1 = srcTri[0].y;x2 = srcTri[1].y;x3 = srcTri[2].y;y1 = srcTri[0].x;y2 = srcTri[1].x;y3 = srcTri[2].x;

				xx1 = howTri[0].y;xx2 = howTri[1].y;xx3 = howTri[2].y;yy1 = howTri[0].x;yy2 = howTri[1].x;yy3 = howTri[2].x;
				M[0] = ((xx1-xx2)*(y2-y3)-(xx2-xx3)*(y1-y2))/((x1-x2)*(y2-y3)-(x2-x3)*(y1-y2));
				M[1] = ((x2-x3)*(xx1-xx2)-(x1-x2)*(xx2-xx3))/((x2-x3)*(y1-y2)-(y2-y3)*(x1-x2));
				M[2] = xx1 - M[0]*x1 - M[1]*y1;
				M[3] = ((yy1-yy2)*(y2-y3)-(yy2-yy3)*(y1-y2))/((x1-x2)*(y2-y3)-(x2-x3)*(y1-y2));
				M[4] = ((yy1-yy2)*(x2-x3)-(yy2-yy3)*(x1-x2))/((y1-y2)*(x2-x3)-(y2-y3)*(x1-x2));
				M[5] = yy1 - M[3]*x1 - M[4]*y1;


				xx1 = desTri[0].y;xx2 = desTri[1].y;xx3 = desTri[2].y;yy1 = desTri[0].x;yy2 = desTri[1].x;yy3 = desTri[2].x;
				V[0] = ((xx1-xx2)*(y2-y3)-(xx2-xx3)*(y1-y2))/((x1-x2)*(y2-y3)-(x2-x3)*(y1-y2));
				V[1] = ((x2-x3)*(xx1-xx2)-(x1-x2)*(xx2-xx3))/((x2-x3)*(y1-y2)-(y2-y3)*(x1-x2));
				V[2] = xx1 - V[0]*x1 - V[1]*y1;
				V[3] = ((yy1-yy2)*(y2-y3)-(yy2-yy3)*(y1-y2))/((x1-x2)*(y2-y3)-(x2-x3)*(y1-y2));
				V[4] = ((yy1-yy2)*(x2-x3)-(yy2-yy3)*(x1-x2))/((y1-y2)*(x2-x3)-(y2-y3)*(x1-x2));
				V[5] = yy1 - V[3]*x1 - V[4]*y1;
	//			int c =0;
				if ((x1>0 && x1<I.cols && y1>0 && y1<I.rows) || (x2>0 && x2<I.cols && y2>0 && y2<I.rows) || (x3>0 && x3<I.cols && y3>0 && y3<I.rows))
				{
					for (vector<Point2f>::iterator it = (*iter).begin(); it!=(*iter).end(); ++it)
					{
						int i = (*it).x;
						int j = (*it).y;
						float r = M[0]*j + M[1]*i + M[2];
						float q = M[3]*j + M[4]*i + M[5];

						float rr = V[0]*j + V[1]*i + V[2];
						float qq = V[3]*j + V[4]*i + V[5];

//						cout << (int)r <<" "<<  (int)q << endl;
	//					e.at<unsigned char>(r, q) =233;
						New.at<Vec3b>(q,r)[0] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[0]) + alpha*(float)(J.at<Vec3b>(qq,rr)[0]);
						New.at<Vec3b>(q,r)[1] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[1]) + alpha*(float)(J.at<Vec3b>(qq,rr)[1]);
						New.at<Vec3b>(q,r)[2] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[2]) + alpha*(float)(J.at<Vec3b>(qq,rr)[2]);
					}
				}
				iter++;
			}
	//		cout << c << endl;
		}
		else
		{
			for (int i = 0; i < I.rows; ++i)
			{
				for (int j = 0; j < I.cols; ++j)
				{
					float rr = V[0]*j + V[1]*i + V[2];
					float qq = V[3]*j + V[4]*i + V[5];

					float r = ((1-alpha)*(float)j + alpha*rr);
					float q = ((1-alpha)*(float)i + alpha*qq);
					if (r>=0 && r<I.cols && q>=0 && q<I.rows && rr>=0 && rr<I.cols && qq>=0 && qq<I.rows)
					{
						New.at<Vec3b>(q,r)[0] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[0]) + alpha*(float)(I.at<Vec3b>(qq,rr)[0]);
						New.at<Vec3b>(q,r)[1] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[1]) + alpha*(float)(I.at<Vec3b>(qq,rr)[1]);
						New.at<Vec3b>(q,r)[2] =(1-alpha)*(float)(I.at<Vec3b>(i, j)[2]) + alpha*(float)(I.at<Vec3b>(qq,rr)[2]);
					}
				}
			}
		}
		for (int i = 0; i < I.rows; ++i)
		{
			for (int j = 0; j < I.cols; ++j)
			{
				New.at<Vec3b>(i,j)[0] = Median(i, j, New, 0);
				New.at<Vec3b>(i,j)[1] =	Median(i, j, New, 1);
				New.at<Vec3b>(i,j)[2] =	Median(i, j, New, 2);
			}
		}
	
		char a = te;
		string r(1,a);
		a=tee;
		string r1(1,a);
		r = red1+"/photo"+r1+r+".jpg";
//		cout << r << endl;
		imwrite(r, New);
		te++;
		if (te == 122)
		{
			te=97;
			tee++;
		}
 	}	

	return 0;
}
