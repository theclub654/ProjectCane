#include "BrxWalker.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

namespace {
void U16(std::vector<uint8_t>& o,uint16_t v){o.push_back(uint8_t(v));o.push_back(uint8_t(v>>8));}
void U32(std::vector<uint8_t>& o,uint32_t v){for(unsigned s=0;s<32;s+=8)o.push_back(uint8_t(v>>s));}
void S32(std::vector<uint8_t>& o,int32_t v){U32(o,uint32_t(v));}
void F32(std::vector<uint8_t>& o,float v){uint32_t b;std::memcpy(&b,&v,4);U32(o,b);}
void ReadSaa(CBinaryInputStream& s){auto t=s.U16Read();if(t==0||t==0xffff)return;if(t>8)throw std::runtime_error("Unknown shader animation type");s.S16Read();s.U16Read();s.Skip(24);}
enum : uint32_t { TextureRgba8, TextureBc1, TextureBc3 };
struct PcImage{uint32_t w{},h{},zone{},mips{},flags{},offset{},format{};std::vector<uint8_t> data;};
std::vector<uint8_t> Mips(std::vector<uint8_t> level,uint32_t w,uint32_t h,bool enabled,uint32_t& count){
 std::vector<uint8_t> out=level;count=1;
 while(enabled&&(w>1||h>1)){uint32_t nw=std::max(1u,w/2),nh=std::max(1u,h/2);std::vector<uint8_t> next(size_t(nw)*nh*4);
  for(uint32_t y=0;y<nh;++y)for(uint32_t x=0;x<nw;++x)for(unsigned c=0;c<4;++c){unsigned sum=0,n=0;for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx){auto sx=std::min(w-1,x*2+dx),sy=std::min(h-1,y*2+dy);sum+=level[(size_t(sy)*w+sx)*4+c];++n;}next[(size_t(y)*nw+x)*4+c]=uint8_t((sum+n/2)/n);}
  out.insert(out.end(),next.begin(),next.end());level=std::move(next);w=nw;h=nh;++count;
 }return out;
}
std::vector<uint8_t> CompressBc(const std::vector<uint8_t>& rgba,uint32_t width,uint32_t height,uint32_t mipCount,uint32_t format){
 std::vector<uint8_t> out;size_t sourceOffset=0;const size_t blockBytes=format==TextureBc1?8:16;
 for(uint32_t level=0;level<mipCount;++level){const uint32_t blocksX=(width+3)/4,blocksY=(height+3)/4;const size_t oldSize=out.size();out.resize(oldSize+size_t(blocksX)*blocksY*blockBytes);uint8_t* destination=out.data()+oldSize;
  for(uint32_t by=0;by<blocksY;++by)for(uint32_t bx=0;bx<blocksX;++bx){uint8_t block[64];for(uint32_t y=0;y<4;++y)for(uint32_t x=0;x<4;++x){const uint32_t sx=std::min(width-1,bx*4+x),sy=std::min(height-1,by*4+y);std::memcpy(block+(y*4+x)*4,rgba.data()+sourceOffset+(size_t(sy)*width+sx)*4,4);}stb_compress_dxt_block(destination,block,format==TextureBc3,STB_DXT_HIGHQUAL);destination+=blockBytes;}
  sourceOffset+=size_t(width)*height*4;width=std::max(1u,width/2);height=std::max(1u,height/2);
 }if(sourceOffset!=rgba.size())throw std::runtime_error("RGBA mip chain size mismatch before BC compression");return out;
}
}

void BrxWalker::LoadShadersFromBrx(CBinaryInputStream& s){
 const auto begin=s.GetOffset();textureTablesBegin_=begin;sourceCluts_.clear();sourceBitmaps_.clear();pendingTextures_.clear();pendingFonts_.clear();
 const auto nc=s.U16Read();sourceCluts_.reserve(nc);for(uint16_t i=0;i<nc;++i){SourceClut c;c.grfzon=s.U32Read();c.colorCount=s.U16Read();c.colorSize=s.U16Read();c.offset=s.U32Read();sourceCluts_.push_back(c);}
 const auto nb=s.U16Read();sourceBitmaps_.reserve(nb);for(uint16_t i=0;i<nb;++i){SourceBitmap b;b.width=s.U16Read();b.height=s.U16Read();b.grfzon=s.U32Read();b.psm=s.S8Read();b.cgsRow=s.S8Read();b.cgsPixels=s.U16Read();b.size=s.U32Read();b.offset=s.U32Read();sourceBitmaps_.push_back(b);}textureTablesEnd_=s.GetOffset();
 const auto ns=s.U16Read();s.U16Read();shaderColors_.assign(ns,{1,1,1,1});
 for(uint16_t si=0;si<ns;++si){auto kind=s.U8Read();s.U8Read();s.S16Read();auto cb=s.GetOffset();std::vector<uint8_t> fc;for(int c=0;c<8;++c){float v=float(s.U8Read())/255.0f;F32(fc,v);if(c<4)shaderColors_[si][c]=v;}RecordReplacement(cb,s.GetOffset(),std::move(fc));s.U32Read();s.U16Read();s.U8Read();auto nt=s.U8Read();ReadSaa(s);
  for(uint8_t ti=0;ti<nt;++ti){PendingTexture p;p.begin=s.GetOffset();p.shaderKind=kind;p.oid=s.S16Read();p.flags=s.S16Read();auto nbi=s.U8Read(),nci=s.U8Read();for(uint8_t i=0;i<nbi;++i)p.bitmaps.push_back(s.U16Read());for(uint8_t i=0;i<nci;++i)p.cluts.push_back(s.U16Read());p.end=s.GetOffset();pendingTextures_.push_back(std::move(p));}}
 const auto nf=s.U16Read();for(uint16_t fi=0;fi<nf;++fi){PendingFont p;p.referenceBegin=s.GetOffset();p.bitmap=s.U16Read();p.clut=s.S16Read();if(p.bitmap>=nb||p.clut<0||p.clut>=nc)throw std::runtime_error("Font texture reference is out of range");pendingFonts_.push_back(p);s.U8Read();s.U8Read();s.U8Read();s.U8Read();s.F32Read();auto ng=s.U32Read();if(ng>0x10000u)throw std::runtime_error("Implausible font glyph count");s.Skip(size_t(ng)*8);}
 Trace("LoadShadersFromBrx",begin,s.GetOffset());
}

void BrxWalker::LoadTexturesFromBrx(CBinaryInputStream& s){
 const auto start=s.GetOffset(),size=s.Remaining();std::array<uint8_t,256> csm{};for(uint16_t i=0;i<256;i+=32)for(uint16_t j=i;j<i+8;++j){csm[j]=uint8_t(j);csm[j+8]=uint8_t(j+16);csm[j+16]=uint8_t(j+8);csm[j+24]=uint8_t(j+24);}
 auto block=[&](uint32_t off,size_t cb){if(off>size||cb>size-off)throw std::runtime_error("Texture source block extends past EOF");std::vector<uint8_t> v(cb);s.Seek(start+off);s.ReadBytes(v.data(),v.size());return v;};
 auto decode=[&](uint16_t bi,uint16_t ci,bool flip){if(bi>=sourceBitmaps_.size()||ci==0xffff||ci>=sourceCluts_.size())throw std::runtime_error("Texture bitmap/CLUT reference is out of range");auto& b=sourceBitmaps_[bi];auto& c=sourceCluts_[ci];auto ix=block(b.offset,b.size);auto pal=block(c.offset,size_t(c.colorCount)*c.colorSize*4);size_t np=size_t(b.width)*b.height;std::vector<uint8_t> rgba(np*4);
  for(size_t p=0;p<np;++p){uint32_t k;if(c.colorCount>16){if(p>=ix.size())throw std::runtime_error("Truncated 8-bit texture");k=csm[ix[p]];}else{if(p/2>=ix.size())throw std::runtime_error("Truncated 4-bit texture");k=(ix[p/2]>>((p&1)*4))&15;}size_t q=size_t(k)*4;if(q+3>=pal.size())throw std::runtime_error("Texture palette index is out of range");rgba[p*4]=pal[q];rgba[p*4+1]=pal[q+1];rgba[p*4+2]=pal[q+2];rgba[p*4+3]=uint8_t(std::min(255u,unsigned(pal[q+3])*255u/128u));}
  if(flip){size_t row=size_t(b.width)*4;for(uint32_t y=0;y<b.height/2;++y)std::swap_ranges(rgba.begin()+y*row,rgba.begin()+(y+1)*row,rgba.begin()+(b.height-1-y)*row);}return rgba;};
 std::vector<PcImage> images;
 // Retail relocation leaves invalid/null texture references unresolved. The
 // renderer then skips that map; it does not attempt to decode it. Preserve
 // that behavior instead of rejecting otherwise valid BRX files.
 // Store only authored level-zero RGBA. The original loader generated world
 // mipmaps through OpenGL after upload; bit 2 requests that same runtime path.
 // Fonts keep their original single-level behavior.
 constexpr uint32_t GenerateMipmaps=1u<<2;
 // Only ordinary world geometry is eligible for lossy block compression.
 // Keep UI-like, generated and effect textures lossless: those categories
 // contain sharp artwork, masks and gradients which expose BC artifacts.
 constexpr uint8_t ShdkThreeWay=0;
 constexpr uint8_t ShdkPrelit=1;
 auto bcEligibleShader=[](uint8_t shaderKind){
  return shaderKind==ShdkThreeWay||shaderKind==ShdkPrelit;
 };
 auto add=[&](uint16_t bi,uint16_t ci,uint32_t flags,bool font,bool allowBc){
  if(bi==0xffff||ci==0xffff||bi>=sourceBitmaps_.size()||ci>=sourceCluts_.size())return int32_t(-1);
  auto& b=sourceBitmaps_[bi];PcImage im;im.w=b.width;im.h=b.height;im.zone=b.grfzon;im.flags=flags|(font?0u:GenerateMipmaps);im.mips=1;
  auto rgba=decode(bi,ci,font);
  if(font||!allowBc){im.format=TextureRgba8;im.data=std::move(rgba);}
  else{
   bool opaque=true;
   for(size_t alpha=3;alpha<rgba.size();alpha+=4)if(rgba[alpha]!=255){opaque=false;break;}
   im.format=opaque?TextureBc1:TextureBc3;
   im.data=CompressBc(rgba,im.w,im.h,im.mips,im.format);
  }
  for(size_t i=0;i<images.size();++i)if(images[i].w==im.w&&images[i].h==im.h&&images[i].zone==im.zone&&images[i].flags==im.flags&&images[i].format==im.format&&images[i].data==im.data)return int32_t(i);
  images.push_back(std::move(im));return int32_t(images.size()-1);
 };
 for(auto& t:pendingTextures_){const bool allowBc=bcEligibleShader(t.shaderKind);std::vector<uint8_t> out;U16(out,uint16_t(t.oid));U16(out,uint16_t(t.flags));U16(out,uint16_t(t.bitmaps.size()));for(size_t f=0;f<t.bitmaps.size();++f){int32_t a=-1,d=-1,z=-1;if(t.shaderKind==ShdkThreeWay){size_t b=f*3;if(b+2>=t.cluts.size())throw std::runtime_error("Three-way texture lacks three CLUTs per frame");a=add(t.bitmaps[f],t.cluts[b],t.flags,false,allowBc);d=add(t.bitmaps[f],t.cluts[b+1],t.flags,false,allowBc);z=add(t.bitmaps[f],t.cluts[b+2],t.flags,false,allowBc);}else{if(f>=t.cluts.size())throw std::runtime_error("Texture lacks a CLUT for one frame");d=add(t.bitmaps[f],t.cluts[f],t.flags,false,allowBc);}S32(out,a);S32(out,d);S32(out,z);}RecordReplacement(t.begin,t.end,std::move(out));}
 for(auto& f:pendingFonts_){std::vector<uint8_t> out;S32(out,add(f.bitmap,uint16_t(f.clut),0,true,false));RecordReplacement(f.referenceBegin,f.referenceBegin+4,std::move(out));}
 std::vector<uint8_t> payload;for(auto& im:images){im.offset=uint32_t(payload.size());payload.insert(payload.end(),im.data.begin(),im.data.end());}
 std::vector<uint8_t> table;U32(table,uint32_t(images.size()));for(auto& im:images){U32(table,im.w);U32(table,im.h);U32(table,im.zone);U32(table,im.mips);U32(table,uint32_t(im.data.size()));U32(table,im.offset);U32(table,im.flags);U32(table,im.format);}RecordReplacement(textureTablesBegin_,textureTablesEnd_,std::move(table));RecordReplacement(start,s.GetSize(),std::move(payload));s.Seek(s.GetSize());Trace("LoadTexturesFromBrx (SHDK-selected RGBA8/BC1/BC3)",start,s.GetOffset());
}
