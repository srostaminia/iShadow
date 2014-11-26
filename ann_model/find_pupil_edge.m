function [lower, upper] = find_pupil_edge(pixels)

    % First do median filtering
    pixels = medfilt1(pixels,3);
    
    % Next, do convolution (with ceiling to cut off specular reflections)
    conv_op = [-1 -1 -1 0 1 1 1];
    conv_op = conv_op / (sum(abs(conv_op(:))));
    conv_offset = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];
    
    edge_detect = abs(conv(pixels, conv_op, 'valid'));    
    
    % Then peak identification
    peaks = find(edge_detect(2:end-1) > edge_detect(3:end) & edge_detect(2:end-1) > edge_detect(1:end-2) & edge_detect(2:end-1) >= 0.48);
    
    % Weed out peaks resulting from specular reflections
    i = 1;
    while i < length(peaks)
        if edge_detect(peaks(i)) > 1
            tmp = find(edge_detect(1:peaks(i) < 1),1,'last');
            
            if isempty(tmp)
                peaks(i) = [];
            else
                peaks(i) = tmp;
                i = i+1; 
            end
            
            if (i < length(peaks))
                tmp = find(edge_detect(peaks(i+1):end) < 1,1);
                
                if isempty(tmp)
                    peaks(i+1:end) = [];
                    break;
                else
                    peaks(i+1) = tmp + peaks(i+1) - 1;
                end
            end
            
        end
        i = i+1;
    end
    
    peaks = [1, peaks, length(edge_detect)];
    
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
    lower = 0; upper = 0;
    for i=1:length(region_means)
        if ( i == 1 || region_means(i) < region_means(i-1) ) && ( i == length(region_means) || region_means(i) < region_means(i+1) )
            if (lower == 0)
                lower = peaks(i);
            end
            
            if peaks(i+1) > upper
                upper = peaks(i + 1);
            end
        end
    end
    
    lower = lower + conv_offset(1);
    upper = upper + conv_offset(1);

end