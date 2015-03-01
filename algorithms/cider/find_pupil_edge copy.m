function edges = find_pupil_edge(start, pixels, peak_thresh, do_round)

    % First do median filtering
    pixels = medfilt1(pixels,3);
    
    % Next, do convolution
    conv_op = [-1 -1 -1 0 1 1 1];
%     conv_op = conv_op / (sum(abs(conv_op(:))));
    conv_offset = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];
    
    if (nargin == 3)
        do_round = 0;
    end
    
    if do_round == 1
        edge_detect = floor(abs(conv(pixels, conv_op, 'valid')));
    else
        edge_detect = abs(conv(pixels, conv_op, 'valid'));
    end
    
    peak_thresh = mean(edge_detect);
    
    % Then peak identification
    peaks = find(edge_detect(2:end-1) > edge_detect(3:end) & edge_detect(2:end-1) >= edge_detect(1:end-2) & edge_detect(2:end-1) >= peak_thresh);
    peaks = peaks + 1;
    
    spec_thresh = 150;
    
    % Weed out peaks resulting from specular reflections
    i = 1;
    specular_regions = [];
    while i < length(peaks)
        if edge_detect(peaks(i)) > 150
            tmp = find(edge_detect(1:peaks(i)) < 150,1,'last');
            
            if isempty(tmp)
                peaks(i) = [];
            else
                peaks(i) = tmp;
                % i+1 to account for the entry we prepend to peaks after this loop
                specular_regions = [specular_regions, i + 1];
                i = i+1; 
            end
            
            if (i <= length(peaks))
                tmp = find(edge_detect(peaks(i):end) < 150,1);
                
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
        edges = [conv_offset(1), length(pixels) - conv_offset(2)];
        return;
    end
    
    % Calculate mean between successive peaks
    region_means = zeros(length(peaks) - 1, 1);
    for i=1:length(peaks) - 1
        region_means(i) = mean(pixels(peaks(i)+1:peaks(i+1)));
    end
    
%     for i=1:length(specular_regions)
%         s_idx = specular_regions(i);
%         if s_idx > 2 && s_idx < (length(peaks) - 1)
%             prev_region = region_means(s_idx - 1);
%             next_region = region_means(s_idx + 1);
%             
%             if prev_region / next_region > 0.6 && prev_region / next_region < 1/0.6
%                 region_means(s_idx:s_idx+1) = [];
%                 peaks(s_idx:s_idx+1) = [];
%             end
%         end
%     end
    
    peak_after = find(peaks > start,1);
    
    if isempty(peak_after)
        peak_after = length(peaks);
    end
    
    local_regions = [];
    if peak_after > 3
        local_regions = [local_regions, peak_after - 2];
    end
    
    local_regions = [local_regions, peak_after - 1];
    
    if peak_after < length(peaks)
        local_regions = [local_regions, peak_after];
    end
    
    [~,sort_idx] = sort(region_means(local_regions));
    min_idx = sort_idx(1);
    edges = [peaks(local_regions(min_idx)), peaks(local_regions(min_idx)+1)];
    
    if ~isempty(find(specular_regions == local_regions(min_idx)-1,1))
        edges = [edges; peaks(local_regions(min_idx)-1), peaks(local_regions(min_idx)+1)];
    end
    
    if ~isempty(find(specular_regions == local_regions(min_idx)+1,1))
        edges = [edges; peaks(local_regions(min_idx)), peaks(local_regions(min_idx)+2)];
    end
    
%     % Identify range of lowest mean(s)
%     rm_localmin = [];
%     rm_non_localmin = [];
%     for i=1:length(region_means)
%         if ( i == 1 || region_means(i) < region_means(i-1) ) && ( i == length(region_means) || region_means(i) < region_means(i+1) )
%             rm_localmin = [rm_localmin, i];
%         else
%             rm_non_localmin = [rm_non_localmin, i];
%         end
%     end
%     
%     threshold = min(region_means(rm_non_localmin));
%     lower = 0; upper = 0;
%     
%     for i=1:length(rm_localmin)
%         if region_means(rm_localmin(i)) < threshold
%             if (lower == 0)
%                 lower = peaks(rm_localmin(i));
%             end
%             
%             if peaks(rm_localmin(i) + 1) > upper
%                 upper = peaks(rm_localmin(i) + 1);
%             end
%         end
%     end
%     
%     edges = [lower, upper];

end