%Select model
load('results/subset_l1_init_strips_k7_lambda0.000100/rep2.mat')
Wfull = these_results.W;

load('results/subset_l1_init_strips_k7_lambda0.004642/rep1.mat')
W10 = these_results.W;

dirs{1} = '../data/addison2/';
dirs{2} = '../data/addison_video/2/';
dirs{3} = '../data/addison_video/3/';
dirs{4} = '../data/addison_video/4/';
dirs{5} = '../data/addison_video/5/';
dirs{6} = '../data/addison_video/6/';
dirs{7} = '../data/addison_video/7/';
dirs{8} = '../data/addison_video/8/';

res = [112,112];

for f=1:8

  if(f>1)
    eye_dir  = sprintf('%seye/',dirs{f});
    out_dir  = sprintf('%sscene/',dirs{f});
    eye_list = dir(sprintf('%seye/*.png',dirs{f}));  
    out_list = dir(sprintf('%sscene/*.png',dirs{f}));  
  elseif(f==1)
    eye_dir  = dirs{f};
    out_dir  = dirs{f};
    eye_list = dir(sprintf('%s*eye.png',dirs{f}));  
    out_list = dir(sprintf('%s*out.png',dirs{f}));  
  end

  writerObj = VideoWriter(sprintf('videos/gaze_video%d.avi',f));
  if(f>1)
    set(writerObj,'FrameRate',12, 'Quality', 90);
  else
    set(writerObj,'FrameRate',4, 'Quality', 90);
  end
  open(writerObj);

  figure(1);colormap(gray);
  set(gcf,'position',[0 0 112*4 112*2]);
  set(gcf,'Renderer','zbuffer');
  set(gca,'nextplot','replacechildren');

  for l=1:length(eye_list);

    tic
    eye_img   = double(imread(sprintf('%s%s',eye_dir,eye_list(l).name)));
    out_img   = imread(sprintf('%s%s',out_dir,out_list(l).name));

    if(f>1)
      eye_img = imresize(eye_img(1:end-40,4:end-1),[112,112]);
    end
        
    pred =  logisticmlp_prediction(W10,[eye_img(:);1]'/255,[0 0],7);
    hold off;
    imagesc([fliplr(eye_img), out_img]);
    hold on;
    plot(112+112*pred(1),112*pred(2),'r+','markersize',20,'linewidth',3);
    axis off;
    drawnow;

    frame = getframe;
    writeVideo(writerObj,frame);

    t=toc;
    %pause(max(0,1/12-t));
  end

  close(writerObj);
end