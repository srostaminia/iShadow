function filtered=plot_vec_filtered(v, conv_op, title_str)

    if nargin < 3
        title_str = '';
    end
    
    conv_op = fliplr(conv_op);
    
    figure;
    subplot(3,1,1);
    
    plot(v);
    title(title_str);
    
    conv_op_adj = conv_op / (sum(abs(conv_op(:))));
    filtered = conv(v, conv_op_adj, 'valid');
    
    coff = [floor(length(conv_op) / 2), ceil(length(conv_op) / 2)];
    subplot(3,1,2);
    plot(coff(1):length(v)-coff(2), abs(filtered));

end