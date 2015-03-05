function q = quick_quantile(data, p)

sorted = sort(data,2);
p_idx = min(ceil(p * size(data,2)), size(data,2));

q = sorted(:,p_idx);