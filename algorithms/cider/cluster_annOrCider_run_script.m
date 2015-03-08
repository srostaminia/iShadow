

% -----------------START OF CONFIGURATION SETTINGS---------------------------------------------------------

% Configure these two values for the experiment
experiment_descriptor = {'irb_radius_target_mean_area'};%{'irb_pupil_percent_0.05','irb_pupil_percent_0-2_98'};
modelNameList={'cider','ann'};
subjects={'addison','duncan','kaituo','malai','mike','shuo'}; %sean
obj='pupil';
lightings={'_calib_'};%,'_dark_'};

uniList = {'uniquefy_0', 'uniquefy_1'};
lambdas_n ={'0.000100','0.000215','0.000464','0.001000','0.002154','0.004642','0.010000','0.021544','0.046416','0.100000'};
% lambdas = [0.001000, 0.010000, 0.100000]

sub_model='';
nDim=3;
scaleVect=[112 111 112];
contrast_method='mean';
percentile=0;
testIndOn=0;
thirdVarName='area';
% -----------------END OF CONFIGURATION SETTINGS-----------------------------------------------------------

names=[];

for i=1:length(lightings)
    names=[names strcat(subjects,lightings{i},obj)];
end

names(strcmp(names,'addison_dark_pupil'))=[];

matlabOn=0;


% % % %

data_names = {'c', 'ind', 'l', 'lambda', 'r', 'these_params', 'these_results'};

work_dir = pwd;

%work/awesomeness_irb_??
path_sh=work_dir;

for exp=1:length(experiment_descriptor)
    
    experiment_title=strcat('awesomeness_',experiment_descriptor{exp});
    path_sh=fullfile(work_dir, experiment_title);
    
    
    
    cd (experiment_title);
    if matlabOn==1
        names=cellstr(ls);
    else
        names=readdir ('.');
        names([1,2])=[];
    end
    
    mkdir('parallelFiles');
    
    
    
    %names(strcmp(names,'parallelFiles'))=[];
    path_sh=fullfile(path_sh,'parallelFiles');
    
    for n=1:length(names)
        name=names{n};
        cd (name);%awesomeness_irb_??/addison_calib_pupil
        
        if matlabOn==1
            uniList=cellstr(ls);
        else
            uniList=readdir ('.');
            uniList([1,2])=[];
        end
        
        %load inputX
        input_dir=fullfile(work_dir,'training_sets',strcat(name,'.mat'));
        
        for u=1%:length(uniList)
            uni=uniList{u};
            cd (fullfile(uni,'results'));%uniquefy_0/results
            
            if matlabOn==1
                lambdas=cellstr(ls);
            else
                lambdas=readdir ('.');
                lambdas([1,2])=[];
            end
            
            for lam=1:length(lambdas)
                lamFile=lambdas{lam};%strcat('subset_l1_init_strips_k7_lambda',lambdas{lam});
                cd (lamFile);
                
                rep_dir=pwd; %currently, in rep file directory
                
                
                for i=1:5
                    fileToSave=sprintf('v7_rep%i.mat',i);
                    rep_file=fileToSave;
                    
                    disp(sprintf('%s - %s - %s - %s', name, uniList{u}, lambdas{lam}, strcat('rep',str(i))));
                  %save v7 file
                    if matlabOn==0
                        if exist(fileToSave, 'file') ~= 2
                            disp({'v7_rep file is already saved before.'});
                        else
                            clear(data_names{:});
                            load(sprintf('rep%i.mat',i));
                            save('-v7', fileToSave, data_names{:});
                            clear(data_names{:});
                            
                        end
                    end
                    
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
                            
                            fprintf(fileID,sprintf('repNum=%i;\n',i));
                            fprintf(fileID,sprintf('modelName=\"%s\";\n',modelName));
                            fprintf(fileID,sprintf('rep_dir=\"%s\";\n',rep_dir));
                            fprintf(fileID,sprintf('input_dir=\"%s\";\n',input_dir));
                            fprintf(fileID,sprintf('do_uniquefy=%i;\n',u));
                            fprintf(fileID,sprintf('nDim=%i;\n',nDim));
                            fprintf(fileID,sprintf('scaleVect=%s;\n',mat2str(scaleVect)));

                            fprintf(fileID,sprintf('sub_model=\"%s\";\n',sub_model));
                            fprintf(fileID,sprintf('contrast_method=\"%s\";\n',contrast_method));
                            fprintf(fileID,sprintf('percentile=%i;\n',percentile));
                            fprintf(fileID,sprintf('testIndOn=%i;\n',testIndOn));
                            fprintf(fileID,sprintf('thirdVarName=\"%s\";\n',thirdVarName));


                            fprintf(fileID,'\n');
                            %fprintf(fileID,'octave --silent --eval "run_model(\"$rep_dir\",\"$input_dir\",\"$repNum\",\"$do_uniquefy\",\"$modelName\",\"$sub_model\",\"$contrast_method\",\"$nDim\",\"$scaleVect\")"');
                            fprintf(fileID,'octave --silent --eval "run_model_cluster(\"$repNum\",\"$modelName\",\"$rep_dir\",\"$input_dir\",\"$do_uniquefy\",\"$nDim\",\"$scaleVect\",\"$sub_model\",\"$contrast_method\",\"$percentile\",\"$testIndOn\",\"$thirdVarName\")"');

                            fclose(fileID);
                            
                            if matlabOn==0
                                command=['qsub', '-cwd', '-o', 'stdout.txt', '-e', 'stderr.txt', data_title,'.sh'];
                                [status,cmdout] = system(command);
                            end
                        end
                         %in parallel file
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

