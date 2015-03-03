function graph_pupil_edges(rows, search_start, q_val)

% quantiles = quick_quantile(rows,q_val);
% rows = bsxfun(@max, rows, quantiles);

figure;

subplot(3,1,1);
imagesc(rows); colormap gray; axis off; axis equal

subplot(3,1,2);

edges = zeros(size(rows,1),2);
for i=1:size(rows,1)
tmp = find_pupil_edge(search_start,rows(i,:),200,1);
edges(i,:) = tmp(end,:);
end

im_edges = zeros(size(rows));
for i=1:size(rows,1)
im_edges([i i], [edges(i,1) edges(i,2)]) = 1;
end

imagesc(im_edges); colormap gray; axis off; axis equal

subplot(3,1,3);

alpha = ones(size(rows)) * 0.5;
imagesc(rows); colormap gray; axis off; axis equal

im_edges(im_edges == 1) = max(rows(:));
im_edges(im_edges == 0) = min(rows(:));
hold on;

h = imagesc(im_edges);
set(h,'AlphaData',alpha);
hold off;