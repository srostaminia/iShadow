% Soha Rostaminia, June 3rd, 2016
% This function makes a mask for 4-column eye images saved in SD card.
% The input should be the images.mat file (:x448), and the output would be the mask image (4 x 112)
% For generating the mask you should face the camera front of the light with a plain sheet covering the camera lens. 
% This function generally takes the 3rd image (this number is optional),and finds the minimum pixel value of that image
% and subtracts it from whole; the final image would be the mask. 
% Now, the mask is for the right-looking image (the upper eyelid above the lower eyelid). 

function [ mask ] = gen_4col_mask( images )
    N = 3;
    if (length(images(:,1)) < 3)
        N = 1;
    end
    img = images(N,:);
    minimum = min(img(:));
    mask = img - minimum;
    mask = reshape(mask, 4, 112);
    mask = fliplr(mask);

end

