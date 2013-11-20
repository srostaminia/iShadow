% stony_apply_mask.m
%
% Russ Bielawski
% 2012-11-02
%

function [mask,pic,rc] = stony_apply_mask(maskfilename,picfilename,flagdisplaypic)
% TODO: make this size agnostic
% TODO: check that maskraw and picraw are the same size
rc=0;
if (2 > nargin) || (3 < nargin)
   rc=-1;
   return;
elseif (2 == nargin)
   flagdisplaypic=1;
end
run(maskfilename);
maskraw=Img;
clear Img;
run(picfilename);
picraw=Img;
clear Img;
mask=maskraw-min(min(maskraw));
pic=255-picraw+mask;
if (1 == flagdisplaypic)
   colormap(gray);
   imagesc(pic);
end

