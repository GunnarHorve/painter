#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>       /* pow */
#include "kmeansSegment.cpp"
#include "drawwindow.cpp"
#include "shapes.cpp"

using namespace cv;
using namespace std;

// ==== HELPER FUNCTIONS =======

// given an image, look for single pixels surrounded by another color... replace by neighbor's
void reduceSpecsInImage(Mat *image,float min_neighbor_percent=.61) {
  Vec3b neighbor_colors[8]; // index lookup of colors, 8=max number of neighbors
  int neighbor_counts[8]; //
  int n_changed = 0; // how many pixels were changed
  int debug = 0;

  for (int i=0; i<image->cols; i++) {
    for (int j=0; j<image->rows; j++) {
      //      if (n_changed>10) { debug = 0; }

      Vec3b color = image->at<Vec3b>(Point(i,j));

      // zero out counts of previous pixel test
      for (int n=0; n<8; n++) { neighbor_counts[n]=0; }
      int n_colors_found=0;
      
      // count how many of each color of the neighbors of i,j
      int left=max(i-1,0);
      int right=min(i+1,image->cols-1);
      int top=max(j-1,0);
      int bottom=min(j+1,image->rows-1);
      int pixels=(1+right-left)*(1+bottom-top) - 1; // how many neighbor pixels to compare
      
      if (debug) printf("[%i,%i](%d,%d,%d) (%d-%d x %d-%d)=%d neighbors\n",i,j,color[0],color[1],color[2],left,right,top,bottom,pixels);

      for (int n=left; n<=right; n++) {
	for (int m=top; m<=bottom; m++) {
	  if (n!=i || m!=j) {
	    // try to look up the color id of the neighbor
	    Vec3b neighbor_color = image->at<Vec3b>(Point(n,m));
	    int color_id=-1;
	    for (int id=0; id<n_colors_found; id++) {
	      if (neighbor_color[0] == neighbor_colors[id][0] && 
		  neighbor_color[1] == neighbor_colors[id][1] && 
		  neighbor_color[2] == neighbor_colors[id][2]) {
		color_id=id; 
	      }
	    }
	    if (color_id == -1) { // neighbor pixel is a new color
	      color_id = n_colors_found;
	      neighbor_colors[color_id] = neighbor_color;
	      n_colors_found++;
	      if (debug) printf(" color[%d,%d] id %d = (%d,%d,%d)\n", n,m,color_id, neighbor_color[0], neighbor_color[1], neighbor_color[2]);
	    }
	    neighbor_counts[color_id]++;
	  }
	}
      }

      // find the max number of a given color
      int max=0;
      int max_color_id=0;
      for (int id=0; id<n_colors_found; id++) {
	if (neighbor_counts[id]>max) { 
	  max=neighbor_counts[id];
	  max_color_id = id; 
	}
	if (debug) printf(" id %d appears %d times\n", id, neighbor_counts[id]);
      }
	
      // if a lot of the neighbors are a different color than i,j, then change the color
      float percent_max = (float)max/pixels;
      if (debug) printf(" max color id %d has %d times (%3.2f percent) = (%d,%d,%d)\n",max_color_id,max,percent_max*100, 
			neighbor_colors[max_color_id][0], neighbor_colors[max_color_id][1], neighbor_colors[max_color_id][2]);

      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      if (percent_max+r*0.15>min_neighbor_percent && (color[0] != neighbor_colors[max_color_id][0] ||
						     color[1] != neighbor_colors[max_color_id][1] ||
						     color[2] != neighbor_colors[max_color_id][2])) {
	if (debug) printf(" - UPDATE color\n");
	// debug
	//neighbor_colors[max_color_id][0]=0;
	//neighbor_colors[max_color_id][1]=255;
	//neighbor_colors[max_color_id][2]=0;
	image->at<Vec3b>(Point(i,j)) = neighbor_colors[max_color_id];
	n_changed++;
      }
    }
  }
  printf("Despecked %d pixels\n",n_changed);
}


// given an image, return a sorted list of colors (darkest first)
void sortColorsInImage(Mat *image, std::vector<Vec3b>& sorted_colors) {
  Vec3b colors[2048]; // max number of colors
  long colors_count[2048] = {0};
  int n_colors=0;

  // scan image and count colors
  for (int i=0; i<image->cols; i++) {
    for (int j=0; j<image->rows; j++) {
      int found=-1;
      //Vec3b color = kmeans_image.at<Vec3b>(Point(i,j));
      Vec3b color = image->at<Vec3b>(Point(i,j));
      for (int k=0; k<n_colors; k++) {
	if (color[0] == colors[k][0] && color[1] == colors[k][1] && color[2] == colors[k][2]) {
	  found=k;
	  colors_count[k]++;
	  continue;
	}
      }
      if (found == -1) {
	colors[n_colors]=color;
	n_colors++;
      }
    }
  }  

  for (int i=0; i<n_colors; i++) {
    //printf("%d: [%d,%d,%d] = %ld NORM:%f\n",i,colors[i][0],colors[i][1],colors[i][2],colors_count[i],norm(colors[i]));
  }

  // create a sorted list
  int* used_colors = new int [n_colors]; // don't forget to delete this later
  for (int i = 0; i < n_colors; i++) { used_colors[i]=0; }

  for (int i=0; i<n_colors; i++) {
    // find the darkest unused color
    int darkest=255*255*255;
    int darkest_index=-1;
    for (int j=0; j<n_colors; j++) {
      if (!used_colors[j] && norm(colors[j])<=darkest) {
	darkest=norm(colors[j]);
	darkest_index=j;
      }
    }
    if (darkest_index>=0) { // should always be true
      used_colors[darkest_index]=1;
      sorted_colors.push_back(colors[darkest_index]);
    }
  }
  delete [] used_colors;

  for (int i=0; i<n_colors; i++) {
    printf("SORTED %d: [%d,%d,%d] = NORM:%f\n",i,sorted_colors[i][0],sorted_colors[i][1],sorted_colors[i][2],norm(sorted_colors[i]));
  }
}


// =============================

// Base class that takes an image and returns drawing data
class ImageParser {
protected:
  int use_random_colors; // mostly for debugging
public:
  void useRandomColors(int r) { use_random_colors=r; }

  int parseImage(Mat image) {
    Size s = image.size();
    printf("parsing image (%i x %i pixels)\n",s.width,s.height);
    return 1;
  }
  ImageParser() { use_random_colors=0; }
};

// Takes an image and returns contour lines via Canny filter
class ImageParserContours: public ImageParser {
protected:
  int min_contour_length;
  int canny_threshold;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  Mat grey_image;

public:
  void setMinContourLength(int len) { min_contour_length = len; }
  void setCannyThreshold(int val) { canny_threshold = val; }

  int parseImage(Mat image) {
    cvtColor(image, grey_image, CV_BGR2GRAY);
    GaussianBlur(grey_image, grey_image, Size(7,7), 1.5, 1.5);
    Canny(grey_image, grey_image, canny_threshold, canny_threshold*2, 3);
    
    /// Show in a window
    namedWindow( "Canny", CV_WINDOW_AUTOSIZE );
    moveWindow("Canny",700,600);
    imshow( "Canny", grey_image );
    
    /// Find contours
    findContours( grey_image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    // only finds the most external contours
    //findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    return 1;
  }
  
  void defineShapes(Shapes *S) {
    // simplify contours
    vector<vector<Point> > contours_poly( contours.size() ); 
    for( int i = 0; i< contours.size(); i++ ) {
      approxPolyDP( Mat(contours[i]), contours_poly[i], 1.1, true ); 
    }

    RNG rng(12345);
    for( int i = 0; i< contours.size(); i++ ) {
      PolyLine *PL = new PolyLine();
      PL->setPenColor(0,0,0);
      if (use_random_colors) { PL->setPenColor(rng.uniform(100,200),rng.uniform(100,200),rng.uniform(100,200)); }

      if (contours_poly[i].size()>min_contour_length) {
	// printf("Contour:%i [%lu points]\n",i,contours_poly[i].size());
	for (int j=0; j<contours_poly[i].size();j++) {
	  //printf("(%i,%i)",contours[i][j].x,contours[i][j].y);
	  //if (j!=contours_poly[i].size()-1) { printf(","); }
	  PL->addPoint(contours_poly[i][j].x,contours_poly[i][j].y);
	}
	S->addShape(PL);
	//printf("\n");
      }
    }
  }

  void draw() {
    // simplified contours
    vector<vector<Point> > contours_poly( contours.size() ); 
    for( int i = 0; i< contours.size(); i++ ) {
      approxPolyDP( Mat(contours[i]), contours_poly[i], 1.1, true ); 
    }
    /// Draw contours
    int num_drawn=0;
    Mat drawing = Mat::zeros( grey_image.size(), CV_8UC3 );
    drawing.setTo(cv::Scalar(200,200,200)); // b,g,r); 

    RNG rng(12345);
    for( int i = 0; i< contours.size(); i++ ) {
      Scalar color = Scalar(255,255,255);
      if (use_random_colors) { color = Scalar(rng.uniform(100,200),rng.uniform(100,200),rng.uniform(100,200)); }
      
      if (contours_poly[i].size()>min_contour_length) {
	num_drawn++;
	drawContours( drawing, contours_poly, i, color, 1, 8, hierarchy, 0, Point() );      
      }
      
      // test printing out some contour points
      if (i%8==3) {
	printf("Contour:%i [%lu points]\n",i,contours_poly[i].size());
	for (int j=0; j<contours_poly[i].size();j++) {
	  printf("(%i,%i)",contours[i][j].x,contours[i][j].y);
	  if (j!=contours_poly[i].size()-1) { printf(","); }
	}
	printf("\n");
      }
    }

    //char text[50];
    //sprintf(text,"Lines: drew %i out of %lu",num_drawn,contours.size());
    //putText(drawing, text, cvPoint(30,30), FONT_HERSHEY_DUPLEX, 0.8, cvScalar(200,200,250), 1, CV_AA);
    printf("Lines: drew %i out of %lu",num_drawn,contours.size());

    namedWindow( "Contour Lines", CV_WINDOW_AUTOSIZE );
    imshow( "Contour Lines", drawing );
    //while (waitKey(33)<0) { }
  }

  ImageParserContours() : ImageParser() {
    min_contour_length = 10;
    canny_threshold = 100;
  }

};


// Takes an image and returns pixel regions
class ImageParserKmeans: public ImageParser {
protected:
  int colors; // number of colors
  int blur_loops; // for kmeans
  int min_region_pixels; // smallest number of pixels to be in a region
  Mat kmeans_image;
  std::vector<std::vector<Point> > regions; // array of arrays of pixels in a region
  std::vector<Vec3b> region_colors;

public:
  void setNumColors(int num) { colors = num; }
  void setBlurLoops(int num) { blur_loops = num; }
  void setMinPixelsInRegion(int min=5) { min_region_pixels=min; }

  int parseImage(Mat image) {
    kmeansSegment kmeans(colors);
    // KMEANS_RANDOM_CENTERS Select random initial centers in each attempt.
    // KMEANS_PP_CENTERS Use kmeans++ center initialization by Arthur and Vassilvitskii [Arthur2007].
    // KMEANS_USE_INITIAL_LABELS During the first (and possibly the only) attempt,
    ///kmeans.set_criteria(KMEANS_RANDOM_CENTERS);
    //blur( image, image, Size(3,3) );
    kmeans_image = kmeans.segment(image);

    // blur and try again... perhaps will make the transitions more smooth
    //result = adaptiveThreshold(result,255,ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,11,2)
    if (blur_loops) {
      for (int i=0; i<2; i++) {
	blur( kmeans_image, kmeans_image, Size(3,3) );
	kmeans_image = kmeans.segment(kmeans_image);
      }
    }
    printf("Colors:%i Blur Loops:%i\n",colors,blur_loops);

    std::vector<Vec3b> colors; // uninitialized.  Let sortColorsInImage fill it.
    sortColorsInImage(&kmeans_image,colors); // prints out which colors are in image
    for (int i=0; i<colors.size(); i++) {
      printf("Defined color #%d: [%d,%d,%d] = brightness norm:%f\n",i,colors[i][0],colors[i][1],colors[i][2],norm(colors[i]));
    }

    /// Show in a window
    namedWindow( "kmeans_window", CV_WINDOW_AUTOSIZE );
    moveWindow("kmeans_window",100,100);
    imshow( "kmeans_window", kmeans_image );

    for (int i=0; i<20; i++) {
      reduceSpecsInImage(&kmeans_image);
    }

    namedWindow( "kmeans_window_despeck", CV_WINDOW_AUTOSIZE );
    moveWindow("kmeans_window_despeck",100,100);
    imshow( "kmeans_window_despeck", kmeans_image );

    defineRegionsInImage(&kmeans_image);
    namedWindow( "kmeans_window_regions", CV_WINDOW_AUTOSIZE );
    moveWindow("kmeans_window_regions",700,100);
    imshow( "kmeans_window_regions", kmeans_image );
    
    return 1;
  }

  // given an image, return a list of contiguous regions in pixels (e.g., if an image has
  // 3 separate regions of red, each region will be a different list of pixels)
  void defineRegionsInImage(Mat *image) {
    RNG rng(12345);
    int debug = 0;
    //Mat done_image = image->clone();
    int rows=image->rows;
    int cols=image->cols;
    int* done = new int [rows*cols]; // which pixels are done
    std::vector<Point> wavefront; // will automatically allocate member if needed
    int num_regions=0;

    for (int i=0; i<rows*cols; i++) { done[i]=-1; }

    std::vector<Vec3b> colors; // uninitialized.  Let sortColorsInImage fill it.
    sortColorsInImage(image,colors); // prints out which colors are in image
    for (int c=0; c<colors.size(); c++) {

      printf("Finding regions of color:(%d,%d,%d)\n",colors[c][0],colors[c][1],colors[c][2]);
      int start_i=0; // where left off looking for a color

      Vec3b set_color;
      if (debug) {
	set_color[0]=rng.uniform(100,200);
	set_color[1]=rng.uniform(100,200);
	set_color[2]=rng.uniform(100,200);
      } else {
	set_color = colors[c];
      }

      int num_regions_for_a_color = 0;
      int found_color=1;
      while (found_color) {
	found_color = 0;
	std::vector<Point> region; // points in this region (auto expands array if needed)

	// find one pixel of this color that hasn't been done
	for (int i=start_i; i<cols; i++) {
	  start_i=i; // for speedup
	  for (int j=0; j<rows; j++) {
	    if (done[i*rows+j] == -1) {
	      Vec3b color = image->at<Vec3b>(Point(i,j));
	      if (color[0] == colors[c][0] && color[1] == colors[c][1] && color[2] == colors[c][2]) {
		if (!found_color) { // *if* added to reduce bugs?
		  //printf(" - first pixel found at pixel %d,%d\n",i,j);
		  wavefront.push_back(Point(i,j)); 
		  region.push_back(Point(i,j)); 
		  num_regions++;
		  num_regions_for_a_color++;
		  done[i*rows+j]=num_regions; // 1=is in wavefront
		  //printf("X:%d %d to %d %d\n",i,j,region[0].x,region[0].y);
		  //printf("X:%d %d to %d %d\n",i,j,wavefront[0].x,wavefront[0].y);
		}
				    
		found_color=1;
		i=image->cols;
		j=image->rows;
	      }
	    }
	  }
	}
      
	if (found_color) { 

	  // expand a wavefront from the found pixel that matches the color
	  // wavefront from that pixel and mark all adjacent ones
	  while (wavefront.size()) {
	    Point p=wavefront[wavefront.size()-1];
	    wavefront.pop_back();

	    // count how many of each color of the neighbors of i,j
	    int left=max(p.x-1 , 0);
	    int right=min(p.x+1 , cols-1);
	    int top=max(p.y-1 , 0);
	    int bottom=min(p.y+1 ,rows-1);
	    //int pixels=(1+right-left)*(1+bottom-top) - 1; // how many neighbor pixels to compare
	  
	    for (int n=left; n<=right; n++) {
	      for (int m=top; m<=bottom; m++) {
		if (n!=p.x && m!=p.y && done[n*rows+m]==-1) { // candidate pixel
		  Vec3b color = image->at<Vec3b>(Point(n,m));
		  if (color[0] == colors[c][0] && color[1] == colors[c][1] && color[2] == colors[c][2]) {
		    done[n*rows+m]=num_regions; // in wavefront
		    wavefront.push_back(Point(n,m)); 
		    region.push_back(Point(n,m)); 
		  }
		}
	      }
	    }
	  }

	  regions.push_back(region);
	  region_colors.push_back(colors[c]);

	  // print out region
	  if (debug) { 
	    printf("REGION %d.%d [%d,%d](%d,%d,%d): %lu points\n",num_regions,num_regions_for_a_color,
		   region[0].x,region[0].y,
		   colors[c][0],colors[c][1],colors[c][2],region.size());
	  }
	  if (region.size()>=min_region_pixels) {
	    for (int i=0; i<region.size(); i++) {
	      if (i<10) {
		if (debug) {
		  printf("(%d,%d)",region[i].x,region[i].y);
		  if (i!=region.size()-1) { printf(","); }
		}
	      }
	      image->at<Vec3b>(Point(region[i].x,region[i].y)) = set_color; // change to xxx
	    }
	    if (debug) printf("\n");
	  } else {
	    Vec3b set_color;
	    set_color[0]=0;
	    set_color[1]=255;
	    set_color[2]=0;
	    for (int i=0; i<region.size(); i++) { image->at<Vec3b>(Point(region[i].x,region[i].y))=set_color; }
	    num_regions--;
	  }
	  while(region.size()) { region.pop_back(); }
	}
      }
    }

    DrawWindow DW = DrawWindow(cols,rows,"Done grid"); // w,h
    DW.clearWindow(255,255,255);
    for (int i=0; i<cols; i++) {
      for (int j=0; j<rows; j++) {
	int d=min(2*done[i*rows+j],255);
	DW.setPenColor(d,d,d);
	if (done[i*rows+j]<0) { DW.setPenColor(0,0,255); }
	else if (done[i*rows+j]<min_region_pixels) { DW.setPenColor(255,0,0); }
	else if (done[i*rows+j]<2*min_region_pixels) { DW.setPenColor(255,255,0); }
	else if (done[i*rows+j]<5*min_region_pixels) { DW.setPenColor(0,255,0); }
	DW.drawPixel(i,j);
      }
    }
    DW.show();
  
    // move a small region's pixels into a larger neighbor
    //void moveSmallRegionsIntoNeighbors() {
    if (0) {
      int num_moved=1;
      int max_loops=10;
      int total_candidate_pixels=0;
      while (num_moved>0 && max_loops>0) {
	num_moved=0;
	max_loops--;
	for (int i=0; i<cols; i++) {
	  for (int j=0; j<rows; j++) {
	    int region_id=done[i*rows+j];
	    int size=regions[region_id].size();
	    if (size<min_region_pixels) { 
	      total_candidate_pixels++;
	      int left=max(i-1,0);
	      int right=min(i+1,image->cols-1);
	      int top=max(j-1,0);
	      int bottom=min(j+1,image->rows-1);
	      int largest_pixels=9999990;
	      int largest_region_id=-1;
	      for (int n=left; n<=right; n++) {
		for (int m=top; m<=bottom; m++) {
		  if (n!=i || m!=j) {
		    int size=regions[done[n*rows+m]].size();
		    if (size>=min_region_pixels && size<largest_pixels) {
		      largest_pixels=size;
		      largest_region_id=done[n*rows+m];
		    }
		  }
		}
	      }
	      if (largest_region_id>=0) { // move this pixel into this region
		num_moved++;
		printf("%i: moved %i,%i from %i(%i pxs) to %i(%i pxs) region\n",total_candidate_pixels,i,j,done[i*rows+j],size,largest_region_id,largest_pixels);
		regions[largest_region_id].push_back(Point(i,j));
		done[i*rows+j]=largest_region_id;

		Vec3b set_color;
		set_color[0]=0;
		set_color[1]=0;
		set_color[2]=255;
		image->at<Vec3b>(j,i)=set_color; 

		// remove the point from the original tiny region
		//printf("BEFORE %lu\n",regions[region_id].size());
		//int remove_index=-1;
		for (int p=0; p<=regions[region_id].size(); p++) {
		  //printf("x=%i y=%i\n",regions[region_id][p].x,regions[region_id][p].y);
		  //if (regions[region_id][p].x==i && regions[region_id][p].y==j) { remove_index=p; }
		}
		//printf("removing %d\n",remove_index);
		//regions[region_id].erase(regions[region_id].begin()+remove_index);
		//continue; //p=regions[region_id].size()+1; // get out of loop so can only remove 1 point from list
		//regions[region_id].erase(std::remove(regions[region_id].begin(), regions[region_id].end(), Point(i,j)), regions[region_id].end());
		//printf("AFTER %lu\n",regions[region_id].size());
	      }
	    }
	  }
	}
      }
      printf("Moved %d pixels to larger regions (loop #%d)\n",num_moved,10-max_loops);
    }
    printf("Defined %d regions\n",num_regions);
    delete [] done;
  }

  void draw() {
    RNG rng(12345);
    DrawWindow W = DrawWindow(kmeans_image.cols,kmeans_image.rows,"Regions"); // w,h
    W.clearWindow(0,255,0);
    for (int r=0; r<regions.size(); r++) {
      int num_pixels=regions[r].size();
      if (num_pixels>=min_region_pixels) {
	W.setPenColor(region_colors[r][2],region_colors[r][1],region_colors[r][0]);
	if (use_random_colors) { W.setPenColor(rng.uniform(100,200),rng.uniform(100,200),rng.uniform(100,200)); }
	printf("%i region: %lu pixels %i,%i,%i\n",r,regions[r].size(),region_colors[r][2],region_colors[r][1],region_colors[r][0]);
	W.drawRegion(regions[r]);
      }
    }
    W.show();
    //while (waitKey(33)<0) { }
  }

  void defineShapes(Shapes *S) {
    RNG rng(12345);
    for (int r=0; r<regions.size(); r++) {      
      int num_pixels=regions[r].size();
      if (num_pixels>=min_region_pixels) {
	PixelRegion *PR = new PixelRegion();
	PR->setPenColor(region_colors[r][2],region_colors[r][1],region_colors[r][0]);
	if (use_random_colors) { PR->setPenColor(rng.uniform(100,200),rng.uniform(100,200),rng.uniform(100,200)); }

	for (int p=0; p<regions[r].size(); p++) {
	  PR->addPoint(regions[r][p].x,regions[r][p].y);
	}     
	S->addShape(PR);
      }
    }
  }

  ImageParserKmeans() : ImageParser() { colors = 10; min_region_pixels = 10; }
};


int main(void)
{
  char* g_image_filename="images/lena.jpg";
  Mat src = imread( g_image_filename, 1 );
  if (src.empty()) {
    std::cout << "!!! Failed imread()\n ./colors <image filename>" << std::endl;
    return -1;
  }  

  DrawWindow WIPC = DrawWindow(src.cols,src.rows,"IPC countours"); // w,h
  Shapes SIPC;
  ImageParserContours IPC;
  if (1) {
    IPC.setMinContourLength(5);
    IPC.setCannyThreshold(50);
    IPC.parseImage(src);
    IPC.useRandomColors(1);
    //IPC.draw(); // 
    IPC.defineShapes(&SIPC);
    //IPC.printImageData(1);
    
    WIPC.clearWindow(230,230,230); // default background is white
    SIPC.drawAll(&WIPC);
    WIPC.show();
  }

  DrawWindow WIPK = DrawWindow(src.cols,src.rows,"IPK pixel regions"); // w,h
  Shapes SIPK;
  ImageParserKmeans IPK;
  if (1) {
    IPK.setMinPixelsInRegion(5);
    IPK.parseImage(src);
    IPK.useRandomColors(1);
    IPK.draw();
    IPK.defineShapes(&SIPK);
    //  IPC.printImageData(1);
    
    WIPK.clearWindow(230,230,230); // default background is white
    SIPK.drawAll(&WIPK);
    WIPK.show();
  }

  while (1) {
    int k = waitKey(33);
    if (k==27) { // Esc key to stop
      return(0);
    }
  }

   return 0;
}
