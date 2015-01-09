% stony_save_mask_h.m
%
% Russ Bielawski
% 2012-11-08
%

function rc=stony_save_mask_h(mask,outfilename,additiveoffset)
% TODO: check that maskraw and picraw are the same size
rc=0;
if (2 > nargin) || (3 < nargin)
   rc=-1;
   return;
elseif (2 == nargin)
   additiveoffset=0;
end
outfile=fopen(outfilename,'w+');
if -1 == outfile
   fprintf('cannot open %s for writing',outfilename);
   rc=-1;
   return;
end
fprintf(outfile,'const char stonymask[112*112]={');
for ii=1:112
   for jj=1:112
      fprintf(outfile,'%d',mask(ii,jj)+additiveoffset);
      if (112 > ii) || (112 > jj)
         fprintf(outfile,',');
      end
   end
end
fprintf(outfile,'};\n');
fclose(outfile);

