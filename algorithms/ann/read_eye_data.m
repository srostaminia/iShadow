function [X,Out]=read_eye_data(data_name, max_idx, min_idx)
    if (nargin < 3)
        min_idx = 0;
    end
    
    X = zeros((max_idx - min_idx), 111 * 112);
    Out = zeros((max_idx - min_idx), 111 * 112);

    x_idx = 1;
    for i=min_idx:max_idx
        X(x_idx,:) = reshape(load(sprintf('~/eye_exper/%s/%s_eye_%06d.txt',data_name,data_name,i)),[1 111*112]);
        Out(x_idx,:) = reshape(load(sprintf('~/eye_exper/%s/%s_out_%06d.txt',data_name,data_name,i)),[1 111*112]);
        x_idx = x_idx + 1;
    end

end