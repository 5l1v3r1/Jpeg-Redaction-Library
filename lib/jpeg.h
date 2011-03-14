// jpeg.h: interface for the Jpeg class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_JPEG
#define INCLUDE_JPEG

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "tiff_ifd.h"

extern int debug;
class Iptc;
class JpegDHT;

class JpegMarker {
 public: 
  // Length is payload size- includes storage for length itself.
  JpegMarker(unsigned short marker, unsigned int location,
	     int length) {
    marker_ = marker;
    location_ = location_;
    length_ = length;
  }
  void LoadFromLocation(FILE *pFile) {
    fseek(pFile, location_ + 4, SEEK_SET);
    LoadHere(pFile);
  }
  void LoadHere(FILE *pFile) {
    data_.resize(length_-2);
    int rv = fread(&data_[0], sizeof(char), length_-2, pFile);
    if (rv  != length_-2) {
      printf("Failed to read marker %x at %d\n", marker_, length_);
      throw("Failed to read marker");
    }
  }
  int Save(FILE *pFile);
  unsigned short slice_;
  // Length is payload size- includes storage for length itself.
  int length_;
  unsigned int location_;
  unsigned short marker_;
  std::vector<char> data_;
};
class Photoshop3Block;
class Jpeg {
public:
  class JpegComponent {
  public:
    JpegComponent(unsigned char *d) {
      id_ = d[0];
      v_factor_ = d[1] & 0xf;
      h_factor_ = d[1] >> 4;
      table_ = d[2];
    }
    void Print() {
      printf("Component %d H %d V %d, Table %d\n",
	     id_, h_factor_, v_factor_, table_);
    }
    int id_;
    int h_factor_;
    int v_factor_;
    int table_;
  };
 Jpeg() :filename_(""), width_(0), height_(0), photoshop3_(NULL) {};
  enum markers { jpeg_soi = 0xFFD8,
		 jpeg_sof0 = 0xFFC0,
		 jpeg_sof2 = 0xFFC2,
		 jpeg_dht = 0xFFC4,
		 jpeg_eoi = 0xFFD9,
		 jpeg_sos = 0xFFDA,
		 jpeg_dqt = 0xFFDB,
		 jpeg_dri = 0xFFDD,
		 jpeg_com = 0xFFDE,
		 jpeg_app = 0xFFE0};
  Jpeg(char const * const pczFilename, bool loadall);
  int LoadFromFile(FILE *pFile, bool loadall, int offset);

  int Save(const char * const filename);
  virtual ~Jpeg();
  const char *MarkerName(int marker) const;
  Iptc *GetIptc();
  // Try to wipe out some of the data.
  // Doesn't work yet.
  void Corrupt(int x, int y, int length) {
    JpegMarker *sos = GetMarker(jpeg_sos);
    if (sos == NULL)
      printf("sos is null\n");
    const int datalen = sos->data_.size();
    char *data = &sos->data_[0];
    float bpp = datalen / (float)(width_ * height_);
    printf("Image is %dx%d. data is %d. Bytes/pixel = %.3f\n", 
	  width_, height_, datalen, bpp);
    int offs = (int)((x + y*height_) * bpp);
    for(int i = 0 ;  i< length; ++i) 
      data[offs+i]=rand() % 255;
  }
    
  ExifIfd *GetExif() {
    for (int i=0; i< ifds_.size(); ++i)
      if (ifds_[i]->GetExif())
	return ifds_[i]->GetExif();
    return NULL;
  }
  std::vector<JpegMarker*> markers_;
  JpegMarker *GetMarker(int marker) {
    for (int i = 0 ; i < markers_.size(); ++i) {
      /* printf("Marker %d is %04x seeking %04x\n", */
      /* 	     i, markers_[i]->marker_, marker); */
      if (markers_[i]->marker_ == marker)
	return markers_[i];
    }
    return NULL;
  }
  // If its a SOF or SOS we pass a slice.
  JpegMarker *AddSOMarker(int marker, int location, int length,
			  FILE *pFile, bool loadall, int slice) {
    // We have true byte length here, but AddMarker needs
    // payload length which assumes there were 2 bytes of length.
    JpegMarker *markerptr = AddMarker(marker, location, length+2,
				      pFile, loadall);
    markerptr->slice_ = slice;
    return markerptr;
  }
  // The length is the length from the file, including the storage for length.
  JpegMarker *AddMarker(int marker, int location, int length,
		 FILE *pFile, bool loadall) {
    JpegMarker *markerptr = new JpegMarker(marker, location, length);
    if (loadall)
      markerptr->LoadHere(pFile);
    else 
      fseek(pFile, location + length + 2, SEEK_SET);
    markers_.push_back(markerptr);
    return markerptr;
  }
  void BuildDHTs(const JpegMarker *dht_block);
  void ParseImage(JpegMarker *sos_block);

  int width_;
  int height_;
  int softype_;
  std::vector<TiffIfd *> ifds_;
  std::string filename_;
  //  unsigned int datalen_, dataloc_;

  unsigned int restartinterval_;

  Photoshop3Block *photoshop3_;
  std::vector<JpegDHT*> dhts_;
  std::vector<JpegComponent*> components_;
};

#endif // INCLUDE_JPEG