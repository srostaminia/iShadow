function c=gaussian_model(X,xy_target,chord_length)
    c = [0, 0];
    r = 0;

    % Calculate row-column (rc) coordinates from xy
    rc_target = [xy_target(2), xy_target(1)];
    rc_target = round(rc_target);
    
    half_chord = ceil((chord_length - 1) / 2);
    
    v_ends = [max([-half_chord + rc_target(1) + 1, 1]), min([half_chord + rc_target(1) + 1, 111])];
    h_ends = [max([-half_chord + rc_target(2) + 1, 1]), min([half_chord + rc_target(2) + 1, 112])];
    
    xy_target = round(xy_target);

    y_ends = [max([-half_chord + xy_target(2) + 1, 1]), min([half_chord + xy_target(2) + 1, 112])];
    x_ends = [max([-half_chord + xy_target(1) + 1, 1]), min([half_chord + xy_target(1) + 1, 111])];
    
    r_ratio = zeros(1,2);

    chords = zeros(4,2);

    chords(1:2,2) = round(xy_target(2));
    chords(3:4,1) = round(xy_target(1));

    y_vec = X( v_ends(1):v_ends(2), rc_target(2) )';
    x_vec = X( rc_target(1), h_ends(1):h_ends(2) );
    x_start = rc_target(2) - h_ends(1);
    y_start = rc_target(1) - v_ends(1);
    
    convop=[1 2 3 2 1];
    convop=convop/sum(convop);
    
    c_idx = [ceil(length(convop) / 2), floor(length(convop) / 2)];
    
    [~,sort_idx] = sort(conv(convop,x_vec(c_idx(1):end-c_idx(2))));
    c(1) = sort_idx(1) + x_ends(1) - 1;
    
    [~,sort_idx] = sort(conv(convop,y_vec(c_idx(1):end-c_idx(2))));
    c(2) = sort_idx(1) + y_ends(1) - 1;
    
    plot(c(1), c(2),'b+','MarkerSize',25,'LineWidth',3);

    return;
end