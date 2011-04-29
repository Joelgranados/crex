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

using namespace cv;
using namespace std;

/*{{{ CroppedImage Class*/
class CroppedImage{
  public:
    CroppedImage ();
    CroppedImage ( const Mat&, const string& );
    bool saveTo ( const string& );
    bool adjustDims ( const int, const int );
    string get_label ();

  private:
    string label;
    Mat image;
};
/*}}} CroppedImage Class*/

/*{{{ VirtualCroppedImageExtractor class*/
struct SimpleAnn {
  Rect rect;
  string label;
};

class VirtualCroppedImageExtractor{
  public:
    VirtualCroppedImageExtractor ();
    VirtualCroppedImageExtractor ( const string&, const string& );
    bool extractCroppedImages ();
    vector<CroppedImage*> getCroppedImages ();
    virtual vector<SimpleAnn> getRectangles () = 0; //overrideMe!
    virtual ~VirtualCroppedImageExtractor () {};

  private:
    Mat img;
    vector<CroppedImage*> croppedImages;
    const string imgfile;

  protected:
    /*Used by virutal getRectangles*/
    const string annfile;
};
/*}}} VirtualCroppedImageExtractor class*/

/*{{{ ITUAnnotationVer1 class*/
class ITUAnnotationVer1 : public VirtualCroppedImageExtractor{
  public:
    ITUAnnotationVer1 ();
    ITUAnnotationVer1 ( const string&, const string& );
    virtual vector<SimpleAnn> getRectangles ();
    virtual ~ITUAnnotationVer1 () {};
};
/* To make a custom implementation of CroppedImageExtractor create an
 * intermediate class like ITUAnnotationVer1 and use typedef to activate it*/
/*}}} ITUAnnotationVer1 class*/

/*{{{ CroppedImageExtractor*/
typedef ITUAnnotationVer1 CroppedImageExtractor;
/*}}} CroppedImageExtractor*/


