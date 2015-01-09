pup_idx = 1;
gp_map = [];

for gaze_idx = 1:size(gaze_eye,1)
    this_gaze = gaze_eye(gaze_idx,:);
    
    if (isequal(pup_eye(pup_idx,:),this_gaze))
        search_result = 1;
    else
        search_result = find(all(bsxfun(@eq,pup_eye(pup_idx:end,:),this_gaze),2));
    end
    
    if (~isempty(search_result))
        gp_map = [gp_map; gaze_idx, search_result + (pup_idx - 1)];
        pup_idx = search_result + pup_idx;
    end
    
    if (mod(gaze_idx,round(0.05 * size(gaze_eye,1))) == 0)
        disp(round(gaze_idx / size(gaze_eye,1) * 100))
    end
end
