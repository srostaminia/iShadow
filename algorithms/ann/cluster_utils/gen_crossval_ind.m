function [train_ind, test_ind] = gen_crossval_ind(X, seeds, do_uniquefy, gout)

train_ind = cell(length(seeds),1);
test_ind = cell(length(seeds),1);

i = 1;
for random_seed=seeds
	rand('seed',random_seed); %set rand seed
	randn('seed',random_seed); %set randn seed

	N = size(X,1);
	Ntest = floor(N*0.2);

	ind = randperm(N);
	test_ind{i} = ind(1:Ntest);

	if do_uniquefy == 1
	    [~, unique_train_ind, ~] = unique(round(gout(ind(Ntest+1:end),:) * 2) / 2, 'rows');
	    train_ind{i} = unique_train_ind(randperm(length(unique_train_ind)));  % Undo the sorting done by the unique function
	else
		train_ind{i} = train_ind; ind(Ntest+1:end);
	end

	i = i + 1;
end