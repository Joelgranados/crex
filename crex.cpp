/* Crop Extractor
 * Copyright (C) 2011 Joel Granados <joel.granados@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include<fstream>
#include "crex.h"

using namespace cv;
using namespace std;

/*{{{ CroppedImage Class */
CroppedImage::CroppedImage ():
  image(Mat::zeros(1,1,CV_8U)),label("") {}

CroppedImage::CroppedImage ( const Mat& image, const string& label ):
  image(image),label(label) {}

bool
CroppedImage::saveTo ( const string& filename )
{
  //FIXME: Does this throw an exception?
  return imwrite(filename,this->image);
}

string
CroppedImage::get_label () {return label;}
/*}}} CroppedImage Class*/

/*{{{ VirtualCroppedImageExtractor Class */
VirtualCroppedImageExtractor::VirtualCroppedImageExtractor ():
  imgfile(""),
  annfile(""),
  img(Mat::zeros(1,1,CV_8U)),
  croppedImages(vector<CroppedImage>()) {}

VirtualCroppedImageExtractor::VirtualCroppedImageExtractor
  ( const string& imgfile, const string& annfile ):
  imgfile(imgfile),
  annfile(annfile),
  img(Mat::zeros(1,1,CV_8U)),
  croppedImages(vector<CroppedImage>())
{
  img = imread(this->imgfile);
  extractCroppedImages ();
}

bool
VirtualCroppedImageExtractor::extractCroppedImages ()
{
  map<string, Rect> rectangles = getRectangles();
  map<string, Rect>::iterator iter;

  this->croppedImages.clear();
  for ( iter=rectangles.begin() ; iter != rectangles.end(); iter++ )
    this->croppedImages.push_back( CroppedImage(Mat(this->img, (*iter).second),
                                   (*iter).first) );
  return true;
}

//FIXME: implement getCroppedImages.
/*}}} VirtualCroppedImageExtractor Class */

/*{{{ ITUAnnotationVer1*/
ITUAnnotationVer1::ITUAnnotationVer1():VirtualCroppedImageExtractor(){}

ITUAnnotationVer1::ITUAnnotationVer1
  ( const string& imgfile, const string& annfile ):
  VirtualCroppedImageExtractor ( imgfile, annfile ){};

map<string, Rect>
ITUAnnotationVer1::getRectangles ()
{
  string line;
  map<string, Rect> rectangles;
  ifstream annfilestream ( this->annfile.data(), ifstream::in );
  int from, to;

  if ( annfilestream.is_open() )
  {
    while ( annfilestream.good() )
    {
      getline(annfilestream, line);

      /* Ignore everything except lines starting with "Bounding"*/
      if ( line.substr(0,8).compare("Bounding") != 0 && line.size() > 75)
        continue;

      // FIXME: Need error checking.
      string label;
      Rect rect;

      /*label*/
      from=27; to=line.find("\" (", 27);
      label = line.substr(from, to);

      /*xmin*/
      from=to; to=from+33;
      istringstream(line.substr(from,to)) >> rect.x;

      /*ymin*/
      from=to+2; to=line.find(") - (", from);
      istringstream(line.substr(from,to)) >> rect.y;

      /*width - xmax*/
      from=to+5; to=line.find(", ", from);
      istringstream(line.substr(from,to)) >> rect.width;
      rect.width = rect.width - rect.x;

      /*height - ymax*/
      from=to+2; to =line.find(")", from);
      istringstream(line.substr(from,to)) >> rect.height;
      rect.height = rect.height - rect.y;

      rectangles[label] = rect;
    }
    annfilestream.close();
  }
  return rectangles;
}

/*}}} ITUAnnotationVer1*/

int
main (){
  CroppedImageExtractor cie("file", "file2");
}
