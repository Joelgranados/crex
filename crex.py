#!/usr/bin/env python
# coding=utf-8
# Crop Extractor
# Copyright (C) 2011 Joel Granados <joel.granados@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
import os
import os.path
import pycrex
import logging
import sys

def crex_dir ( from_dir, to_dir=None, ext="png", new_size=None, margin=0 ):
    """Extract annotated images.
    from_dir = Where to look for the images and annotations.
    to_dir = Where to put the crops.  Defaults to from_dir.
    ext = Extension used to save the crops.
    new_size = Size normalization param. (width, height)
    """

    if to_dir==None:
        to_dir = from_dir

    if not os.path.isdir(from_dir):
        raise CREXDirException(from_dir)
    if not os.path.isdir(to_dir):
        raise CREXDirException(to_dir)

    ### CREATE ann_pairs ###
    ann_pairs = [] #[(file, anno), (file, anno)...]
    for entry in os.listdir(from_dir):
        ann_file = os.path.join(from_dir,entry)
        if os.path.isdir( ann_file ) \
           or not ann_file.endswith(".ann"):
            continue

        # here, ann_file ends with ".ann" and is a file.
        img_file = os.path.splitext(ann_file)[0] #img_file contains full path

        if not os.path.exists(img_file) or os.path.isdir(img_file):
            continue #.ann file does not have a pair

        ann_pairs.append( (img_file, ann_file) )

    ### DO THE CREX ###
    npf = NumberedPathFactory(to_dir, ext=ext)
    for ann_pair in ann_pairs:
        (img_file, ann_file) = ann_pair
        crexlog.debug("crexing %s" % ann_file)
        crop_imgs = pycrex.get_cropped_images(img_file, ann_file, margin)
        for crop_img in crop_imgs:
            if not new_size is None and type(new_size) is tuple \
                    and len(new_size) == 2:
                crop_img.adjust_dims( new_size[0], new_size[1] )

            crop_img.save_to( npf.get_output_path(crop_img.get_label()) )

class NumberedPathFactory:
    #top_dir = None
    #label_ref = None

    def __init__(self, top_dir, ext="png" ):
        self.top_dir = top_dir
        self.label_ref = {}
        self.ext = ext

    def get_output_path(self, label):
        label_dir = os.path.join (self.top_dir, label)
        if not label in self.label_ref.keys():#create new label entry.
            if os.path.isdir(label_dir):
                #Search for label offset in label_dir
                clo = -1 #Current Label Offset. -1 in case there are no files
                for entry in os.listdir(label_dir):
                    if os.path.isdir( os.path.join(label_dir,entry) ):
                        continue

                    try:
                        # Assume info is between ^ and first "."
                        tlo = int(entry.split(".")[0]) #Temp Label Offset
                    except:
                        continue
                    if tlo > clo:
                        clo = tlo
                self.label_ref[label] = self.pfn(clo+1)

            else:
                os.mkdir(label_dir)
                self.label_ref[label] =  self.pfn(0)

        retval = os.path.join(label_dir,self.label_ref[label])
        self.label_ref[label] = self.pfn( int(self.label_ref[label]) + 1 )
        return retval+"."+self.ext


    # Pretty Format Number.
    def pfn(self, number, digits=6):
        return ((digits*"0"+"%d")%number)[-digits:]

class CREXException(Exception):
    message = "Unknown error"
    def __init__(self):
        pass
    def __str__(self):
        return ("CREX Error: %s" % self.message)
class CREXDirException(CREXException):
    def __init__(self, dir_name):
        self.message = "Directory %s not found"%dir_name

def initLogger():
    Logger = logging.getLogger("crex")
    Logger.setLevel(logging.DEBUG)
    handler = logging.StreamHandler(sys.stdout)

    #FIXME: Add a file log when we add the config file.
    #handler = logging.FileHandler(config.log.filename)

    formatter = logging.Formatter("%(asctime)s -%(levelname)s - %(message)s")
    handler.setFormatter(formatter)
    Logger.addHandler(handler)

initLogger()
crexlog = logging.getLogger("crex")

