for i = 1:size(Out,1)
    if (gout(i,1) > 53) && (gout(i,1) < 57) && (gout(i,2) > 53) && (gout(i,2) < 57)
        disp(i)
    end
end