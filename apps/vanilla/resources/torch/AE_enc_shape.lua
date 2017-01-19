function AE_enc_shape(input, desc_size)

	--mnist = require 'mnist'
	require 'cunn'
	require 'xlua'
	require 'optim'
	require 'nn'
	require 'dpnn'
	c = require 'trepl.colorize'
	require 'image'

	cmd_params={}
	----------------------------------
	cmd_params.backend = 'nn'
	cmd_params.type = 'cuda'
	----------------------------------
	cmd_params.gpumode = 1
	cmd_params.gpu_setDevice = 1


	function cast(t)
	   if cmd_params.type == 'cuda' then
	      require 'cunn'
		gpumode = cmd_params.gpumode
		if gpumode==1 then
		    cutorch.setDevice(cmd_params.gpu_setDevice)
		end
	      return t:cuda()
	   elseif cmd_params.type == 'float' then
	      return t:float()
	   elseif cmd_params.type == 'cl' then
	      require 'clnn'
	      return t:cl()
	   else
	      error('Unknown type '..cmd_params.type)
	   end
	end


	local seed = 1234567890
	torch.manualSeed(seed)


	-- #Load the model file
	model_wts = torch.load('models/model_'.. desc_size.. '.net')

	model = nn.Sequential()
	model:add(cast(nn.Copy('torch.FloatTensor', torch.type(cast(torch.Tensor())))))
	model:add(cast(model_wts))
	model:get(1).updateGradInput = function(input) return end
	if cmd_params.backend == 'cudnn' then
	   require 'cudnn'
	   cudnn.convert(model:get(2), cudnn)
	end
	params, grad_params = model:getParameters()


	-- #Forward pass through the DECODER
	encoding = input:clone()
	bin_mask = model:get(2):get(3):forward(encoding)

	-- #Reshape the output into a vector and return
	bin_mask_vec = nn.Reshape(bin_mask:size(2)*bin_mask:size(3))
	itorch.image(bin_mask)


return enc_vec
