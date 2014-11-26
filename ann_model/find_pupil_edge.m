function [lower, upper] = find_pupil_edge(pixels, peak_thresh)

    % First do median filtering
    pixels = medfilt1(pixels,3);
    
    % Next, do convolution (with ceiling to cut off specular reflections)
    conv_op = [-1 -1 -1 0 1 1 1];
    conv_op = conv_op / (sum(abs(conv_op(:))));
    conv_offset = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];
    
    edge_detect = abs(conv(pixels, conv_op, 'valid'));    
    
    % Then peak identification
    peaks = find(edge_detect(2:end-1) > edge_detect(3:end) & edge_detect(2:end-1) > edge_detect(1:end-2) & edge_detect(2:end-1) >= peak_thresh);
    peaks = peaks + 1;
    
    % Weed out peaks resulting from specular reflections
    i = 1;
    while i < length(peaks)
        if edge_detect(peaks(i)) > 1
            tmp = find(edge_detect(1:peaks(i)) < 1,1,'last');
            
            if isempty(tmp)
                peaks(i) = [];
            else
                peaks(i) = tmp;
                i = i+1; 
            end
            
            if (i <= length(peaks))
                tmp = find(edge_detect(peaks(i):end) < 1,1);
                
                if isempty(tmp)
                    peaks(i:end) = [];
                    break;
                else
                    peaks(i) = tmp + peaks(i) - 1;
                end
            end
            
        end
        i = i+1;
    end

    peaks = peaks + conv_offset(1);
    peaks = [1, peaks, length(pixels)];
    
    if (length(peaks) == 2)
        lower = conv_offset(1); upper = length(pixels) - conv_offset(2);
        return;
    end
    
    % Calculate mean between successive peaks
    region_means = zeros(length(peaks) - 1, 1);
    for i=1:length(peaks) - 1
        region_means(i) = mean(pixels(peaks(i)+1:peaks(i+1)));
    end
    
    % Identify range of lowest mean(s)
    rm_localmin = [];
    rm_non_localmin = [];
    for i=1:length(region_means)
        if ( i == 1 || region_means(i) < region_means(i-1) ) && ( i == length(region_means) || region_means(i) < region_means(i+1) )
            rm_localmin = [rm_localmin, i];
        else
            rm_non_localmin = [rm_non_localmin, i];
        end
    end
    
    threshold = min(region_means(rm_non_localmin));
    lower = 0; upper = 0;
    
    for i=1:length(rm_localmin)
        if region_means(rm_localmin(i)) < threshold
            if (lower == 0)
                lower = peaks(rm_localmin(i));
            end
            
            if peaks(rm_localmin(i) + 1) > upper
                upper = peaks(rm_localmin(i) + 1);
            end
        end
    end

end