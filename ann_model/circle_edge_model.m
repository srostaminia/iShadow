function [c,r]=circle_edge_model(X,xy_target,chord_length)
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
    
    v_line = X( v_ends(1):v_ends(2), rc_target(2) )';
    h_line = X( rc_target(1), h_ends(1):h_ends(2) );
    
    v_pupil = find_pupil_edge(v_line);
    h_pupil = find_pupil_edge(h_line);

    % Calculate chord midpoints to get pupil center
    x_mid = mean(h_pupil) + some_offset;
    y_mid = mean(v_pupil) + some_other_offset;

    c = [x_mid, y_mid];

    % Calculate pupil radius from each chord separately
    r1 = sqrt((x_mid - chords(1,1)) ^ 2 + (y_mid - chords(1,2)) ^ 2);
    r2 = sqrt((y_mid - chords(3,2)) ^ 2 + (x_mid - chords(3,1)) ^ 2);

    if (abs(chords(1,1) - chords(2,1)) < 4 || abs(chords(3,2) - chords(4,2)) < 4)
        return;
    end

    if abs(r1 / r2) < 0.6 || abs(r1 / r2) > 1/0.6
        return;
    end

    r = mean([r1, r2]);

    [circX, circY] = gen_circle(x_mid, y_mid, r);
    plot(circX, circY, 'b-','LineWidth',3);
    plot(x_mid, y_mid, 'b+', 'MarkerSize', 25,'LineWidth',3);

    return;

%     chords
%     thresh
%     r_ratio
%     input('Press enter to continue','s');
    return;
end