require 'cunn'
require 'xlua'
require 'optim'
require 'nn'
require 'dpnn'
require 'image'


function init(desc_size, modelDir)

	cmd_params={}
	----------------------------------
	cmd_params.backend = 'nn'
	cmd_params.type = 'cuda'
	----------------------------------
	function cast(t)
	   if cmd_params.type == 'cuda' then
	      require 'cunn'
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
	model_wts = torch.load(modelDir.. '/model_'.. desc_size.. '.net', 'ascii')

	model = nn.Sequential()
	model:add(cast(nn.Copy('torch.FloatTensor', torch.type(cast(torch.Tensor())))))
	model:add(cast(model_wts))
	model:get(1).updateGradInput = function(input) return end
	if cmd_params.backend == 'cudnn' then
	   require 'cudnn'
	   cudnn.convert(model:get(2), cudnn)
	end
end



function AE_shape_enc(input, desc_size)

	-- #Forward pass through the ENCODER
	local bin_mask = torch.rand(4096):fill(0)
	for k,v in pairs(input) do
		bin_mask[k] = v
	end
	local recons = model:forward(bin_mask)

	-- #Reshape the output into a vector and return
	local enc_vec = model:get(2):get(2).output
	local output = {}
	output = torch.totable(enc_vec)
--[[
	for index = 1,enc_vec:size(1) do
		output[index] = enc_vec[index]
	end
--]]
	return output
end




function AE_enc_shape(input, desc_size)
	-- #Forward pass through the DECODER
	local encoding = torch.rand(desc_size):fill(0)
	for k,v in pairs(input) do
		encoding[k]=v
	end
	encoding = encoding:cuda()
	local bin_mask = model:get(2):get(3):forward(encoding)
	-- #Reshape the output into a vector and return
	local bin_mask_vec = nn.Reshape(bin_mask:size(1)*bin_mask:size(2)):cuda():forward(bin_mask)
	
	local output = {}
	output = torch.totable(bin_mask_vec)
--[[
	for index = 1,bin_mask_vec:size(1) do
		output[index]=bin_mask_vec[index]
	end
--]]
	return output
end
