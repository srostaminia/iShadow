

% -----------------START OF CONFIGURATION SETTINGS---------------------------------------------------------

% Configure these two values for the experiment
experiment_descriptor = {'irb_pupil_percent_0.05','irb_pupil_percent_0-2_98'};

modelNameList={'cider','ann'};
subjects={'addison'};%['duncan','kaituo','malai','mike'] %sean
obj='pupil';
lightings={'_calib_','_dark_'};

sub_ann='';
nDim=3;
scaleVect=[112 111 112];%[112 111];
contrast_method='mean';

uniList = {'uniquefy_0', 'uniquefy_1'};
lambdas_n ={'0.000100','0.000215','0.000464','0.001000','0.002154','0.004642','0.010000','0.021544','0.046416','0.100000'};
% lambdas = [0.001000, 0.010000, 0.100000]



% -----------------END OF CONFIGURATION SETTINGS-----------------------------------------------------------

names=[];

for i=1:length(lightings)
    names=[names strcat(subjects,lightings{i},obj)];
end

names(strcmp(names,'addison_dark_pupil'))=[];

% % % %

data_names = {'c', 'ind', 'l', 'lambda', 'r', 'these_params', 'these_results'};

work_dir = pwd;

%work/awesomeness_irb_??
path_sh=work_dir;

for exp=1:length(experiment_descriptor)
    
    experiment_title=strcat('awesomeness_',experiment_descriptor{exp});
    path_sh=fullfile(work_dir, experiment_title);
    
    
    
    cd (experiment_title);
    names=cellstr(strsplit(ls));
    %names=readdir ('.');
    %names([1,2])=[];
    
    mkdir('parallelFiles');
    
    
    
    %names(strcmp(names,'parallelFiles'))=[];
    path_sh=fullfile(path_sh,'parallelFiles');
    
    for n=1:length(names)-1
        name=names{n};
        cd (name);%awesomeness_irb_??/addison_calib_pupil
        uniList=cellstr(strsplit(ls));
        %uniList=readdir ('.');
        %uniList([1,2])=[];
        
        %load inputX
        input_dir=fullfile(work_dir,'training_sets',strcat(name,'.mat'));
        
        for u=1:length(uniList)-1
            uni=uniList{u};
            cd (fullfile(uni,'results'));%uniquefy_0/results
            lambdas=cellstr(strsplit(ls));
            %lambdas=readdir ('.');
            %lambdas([1,2])=[];
            
            for lam=1:length(lambdas)-1
                lamFile=lambdas{lam};%strcat('subset_l1_init_strips_k7_lambda',lambdas{lam});
                cd (lamFile);
                
                rep_dir=pwd; %currently, in rep file directory
                
                
                for i=1:5
                    fileToSave=sprintf('v7_rep%i.mat',i);
                    rep_file=fileToSave;
                    
                    %save v7 file
%                     if exist(fileToSave, 'file') == 2
%                         clear(data_names{:});
%                         load(sprintf('rep%i.mat',i));
%                         save('-v7', fileToSave, data_names{:});
%                         clear(data_names{:});
%                         
%                         disp({'ERROR: already saved before.'});
%                         exit;
%                     else
%                         disp(sprintf('%s - %s - %s - %s', name, uniList{u}, lambdas{lam}, strcat('rep',i)));
%                     end
                    
                    %run model
                    
                    
                    cd(path_sh);
                    
                    
                    for ind=1:length(modelNameList)
                        modelName= modelNameList{ind};
                        
                        sub=strcat(name,'_',experiment_descriptor{exp},'_',uniList{u}, '_',lambdas_n{lam},'_',modelName,'_rep');
                        data_title = sprintf('%s%d',sub,i);
                        
                        %mkdir(data_title);
                        %cd(data_title);
                        
                        modelFileName=sprintf('%s_rep%i',modelName,i);
                        
                        
                        
                        
                        if exist(modelFileName, 'file') ~= 2
                            fileID = fopen(fullfile(path_sh , strcat(data_title ,'.sh')),'w');
                            fprintf(fileID,'#!/bin/bash\n');
                            
                            fprintf(fileID,sprintf('cd %s/run_model\n',work_dir));
                            fprintf(fileID,'sub_model=\"\";');
                            fprintf(fileID,'nDim=3;');
                            fprintf(fileID,'scaleVect=[112 111 112];');
                            fprintf(fileID,'contrast_method=\"mean\";');
                            fprintf(fileID,sprintf('modelName=\"%s\";',modelName));
                            fprintf(fileID,sprintf('input_dir=\"%s\";',input_dir));
                            fprintf(fileID,sprintf('rep_dir=\"%s\";',fullfile(rep_dir,sprintf('v7_rep%i.mat',i))));
                            
                            fprintf(fileID,'\n');
                            fprintf(fileID,'octave --silent --eval "run_model(\"$rep_dir\",\"$input_dir\",\"$repNum\",\"$do_uniquefy\",\"$modelName\",\"$sub_model\",\"$contrast_method\",\"$nDim\",\"$scaleVect\")"');
                            
                            fclose(fileID);
                            
                            %command=['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title,'.sh'];
                            %[status,cmdout] = system(command);
                        end
                        %cd('..'); %in parallel file
                    end
                    cd(rep_dir); %in one lambda folder
                end
                cd('..'); %all lambdas
            end
            cd('../..')% all unique
        end
        cd('..')% all subject file
    end
    cd('..')%all irb file
end
cd(work_dir);

