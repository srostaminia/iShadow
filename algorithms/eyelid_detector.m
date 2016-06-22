% Soha Rostaminia, June 3rd, 2016
% This function takes the denoised image and detects the upper eyelid location.
% The input should be a 4-col image (4x112) (the upper eyelid should be above the lower one), 
% and the output would be the y-axis value of the upper eyelid location. 
% Default values for pupil_diameter = 25, and up_eyelid_thresh = 20;

function [ LEyelid, pupil_index ] = eyelid_detector( image, prev_eyelid, pupil_diameter, up_eyelid_thresh)
    pupil_index = 0;
    % Sums all 4 values of columns in each row for denoising
    intimage = sum(image);
    % Uses 1D median filter for smoothing
    MedFil = medfilt1(intimage, 5);
    % Find the edges
    c = [-1 -1 -1 0 1 1 1];
    y = conv(MedFil, c, 'valid');
    % The negative outputs are out of my interest (they mean that there is an edge from high intensity to lower one, 
    % but I would like to find the eyelid so understanding the edges from low to high intensity would be enough.
    y(y < 0) = 0;
    edge_detect = y;
    peak_thresh = mean(edge_detect);
    peaks = find(edge_detect(2:end-1) > edge_detect(3:end) & edge_detect(2:end-1) >= edge_detect(1:end-2) & edge_detect(2:end-1) >= peak_thresh);
    peaks = peaks + 1; 
    
    % Weed out peaks resulting from specular reflections
    spec_thresh = 250;
    peaks(edge_detect(peaks) > spec_thresh) = [];
    
    % Flitering out the outliers (The value of this should come from the training data - this line means that the eyelid could not be very high)
    peaks = peaks(peaks(:) > up_eyelid_thresh);
    
    if (length(peaks) > 1)
        
        if (length(peaks) > 2)
            % Filtering out the peaks with low probability of being eyelid
            if (abs(mean(edge_detect(peaks)) - min(edge_detect(peaks))) > 10)
                peaks(edge_detect(peaks) < mean(edge_detect(peaks))) = [];
            end
        end
        MaxPeak = find(edge_detect(peaks) > (.7*(max(edge_detect(peaks))-min(edge_detect(peaks)))) + min(edge_detect(peaks)));

        if (isempty(MaxPeak) | (edge_detect(peaks(MaxPeak)) - mean(edge_detect(peaks)) < 20))
            MaxPeak = 1;
        end
        
        if (length(peaks) > MaxPeak(1))
            if (peaks(MaxPeak(1)+1) - peaks(MaxPeak(1)) < pupil_diameter)
                pupil_index = 1;
            end
        end
    else   
        MaxPeak = 1;
    end
    % Checks if the new estimated eyelid location is not very far from the previous one.  
    if (isempty(prev_eyelid))
        LEyelid = peaks(MaxPeak(1))+6;
    else
        near = find(abs(peaks(MaxPeak)-prev_eyelid) == min(abs(peaks(MaxPeak)-prev_eyelid)));
        LEyelid = peaks(MaxPeak(near(1)))+6;
    end
    
    LEyelid = 112 - LEyelid;

end

