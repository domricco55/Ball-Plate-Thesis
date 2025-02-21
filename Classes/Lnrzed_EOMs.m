classdef Lnrzed_EOMs < handle
    %Lnrze_EOMs Summary of this class goes here
    %   Detailed explanation goes here
    
    properties (SetAccess = private)
        %Ball Plate System Objects
        VDefs
        BP_Kinetics
        
        %8th order system
        y_vec
        C
        D
        C_obs
        D_obs
        A
        B
        J_states
        J_inputs
        TF
        norm_num_1
        norm_den_1
        norm_num_2
        norm_den_2
        norm_num_3
        norm_den_3
        norm_num_4
        norm_den_4
        norm_num_5
        norm_den_5
        norm_num_6
        norm_den_6
        norm_num_7
        norm_den_7
        norm_num_8
        norm_den_8

        %Two Uncoulpled 4th order systems
        A1
        B1
        A2
        B2
        Lin_EOMs
        Lin_EOMs1
        Lin_EOMs2

        %Output y_1 = x + a*beta
        TF_y1
        C_y1
        D_y1
    end
    
    methods
        function obj = Lnrzed_EOMs(Var_Defs, BP_Kinetics)
            %Lnrzed_EOMs Construct an instance of this class
            %   Detailed explanation goes here
            
            
            %Assign the inputs to the constructor to the class properties
            obj.VDefs = Var_Defs;          
            obj.BP_Kinetics = BP_Kinetics;
            
        end 
        
        function [A, B, y_vec, C, D, C_obs, D_obs, TF, Lin_EOMs] = Derive_8th_Ordr_Lin_Sys(obj)
            %Derive_8th_Ordr_Lin_Sys Linearize the NL EOMS and produce an 8th order linear
            %system of equations representing the dynamics near the unstable equilibrium
            %point
            %   Detailed explanation goes here
            
            
            %Derive the linearized 8th order equations of motion
            J_states_fun(obj.VDefs.stateVec) = jacobian(rhs(obj.BP_Kinetics.NL_NumEOMs),...
                obj.VDefs.stateVec); %Take Jacobian WRT States
            obj.J_states = J_states_fun;
            J_inputs_fun(obj.VDefs.stateVec) = jacobian(rhs(obj.BP_Kinetics.NL_NumEOMs),...
                obj.VDefs.inputVec); %Take Jacobian WRT Inputs
            obj.J_inputs = J_inputs_fun;
            
            %State Coupling and Input Matrices from evaluation of the Jacobians at the
            %unstable equilibrium point
            obj.A = obj.J_states(0,0,0,0,0,0,0,0);
            obj.B = obj.J_inputs(0,0,0,0,0,0,0,0);

            
            %Define these symbolic quantities for use in further analysis:
            obj.y_vec = [obj.VDefs.x obj.VDefs.y obj.VDefs.beta obj.VDefs.gamma].';
            obj.C = sym([1 0 0 0 0 0 0 0;0 0 1 0 0 0 0 0;0 0 0 0 1 0 0 0;0 0 0 0 0 0 1 0]);
            obj.D = sym(zeros(4,2));  
            
            %Get system transfer function
            obj.C_obs = eye(8);
            obj.D_obs = sym(zeros(8,2));
            obj.TF = obj.C_obs*(obj.VDefs.s*eye(8) - obj.A)\obj.B + obj.D_obs; 

            %Get transfer function coefficients
                %x direction
            [n1,d1] = numden((obj.TF(1,1)));
            [n2,d2] = numden((obj.TF(2,1)));
            n2 = expand(n2);
            [n3,d3] = numden((obj.TF(3,1)));
            [n4,d4] = numden((obj.TF(4,1)));
            
            num_coeffs1 = coeffs(n1,obj.VDefs.s,'All');
            num_coeffs2 = coeffs(n2,obj.VDefs.s,'All');
            num_coeffs3 = coeffs(n3,obj.VDefs.s,'All');
            num_coeffs4 = coeffs(n4,obj.VDefs.s,'All');          
            den_coeffs1 = coeffs(d1,obj.VDefs.s,'All');
            den_coeffs2 = coeffs(d2,obj.VDefs.s,'All');
            den_coeffs3 = coeffs(d3,obj.VDefs.s,'All');
            den_coeffs4 = coeffs(d4,obj.VDefs.s,'All');

            obj.norm_num_1 = double(num_coeffs1/den_coeffs1(1));
            obj.norm_den_1 = double(den_coeffs1/den_coeffs1(1));
            obj.norm_num_2 = double(num_coeffs2/den_coeffs2(1));
            obj.norm_den_2 = double(den_coeffs2/den_coeffs2(1));
            obj.norm_num_3 = double(num_coeffs3/den_coeffs3(1));
            obj.norm_den_3 = double(den_coeffs3/den_coeffs3(1));
            obj.norm_num_4 = double(num_coeffs4/den_coeffs4(1));
            obj.norm_den_4 = double(den_coeffs4/den_coeffs4(1));
            
                %y direction
            [n5,d5] = numden((obj.TF(5,2)));
            [n6,d6] = numden((obj.TF(6,2)));
            n6 = expand(n6);
            [n7,d7] = numden((obj.TF(7,2)));
            [n8,d8] = numden((obj.TF(8,2)));
            
            num_coeffs5 = coeffs(n5,obj.VDefs.s,'All');
            num_coeffs6 = coeffs(n6,obj.VDefs.s,'All');
            num_coeffs7 = coeffs(n7,obj.VDefs.s,'All');
            num_coeffs8 = coeffs(n8,obj.VDefs.s,'All');  
            den_coeffs5 = coeffs(d5,obj.VDefs.s,'All');
            den_coeffs6 = coeffs(d6,obj.VDefs.s,'All');
            den_coeffs7 = coeffs(d7,obj.VDefs.s,'All');
            den_coeffs8 = coeffs(d8,obj.VDefs.s,'All');
            
            obj.norm_num_5 = double(num_coeffs5/den_coeffs5(1));
            obj.norm_den_5 = double(den_coeffs5/den_coeffs5(1));
            obj.norm_num_6 = double(num_coeffs6/den_coeffs6(1));
            obj.norm_den_6 = double(den_coeffs6/den_coeffs6(1));
            obj.norm_num_7 = double(num_coeffs7/den_coeffs7(1));
            obj.norm_den_7 = double(den_coeffs7/den_coeffs7(1));
            obj.norm_num_8 = double(num_coeffs8/den_coeffs8(1));
            obj.norm_den_8 = double(den_coeffs8/den_coeffs8(1));



            %Create symbolic equation object for the total system (used later in
            %control system design)
            obj.Lin_EOMs = obj.A*obj.VDefs.stateVec + obj.B*[obj.VDefs.T_beta;obj.VDefs.T_gamma];

           %Return outputs
            A = obj.A;
            B =obj.B;
            y_vec = obj.y_vec;
            C = obj.C;
            D = obj.D;
            C_obs = obj.C_obs;
            D_obs = obj.D_obs;
            Lin_EOMs = obj.Lin_EOMs;
            TF = obj.TF;
        end
        
        function [A1, B1, A2, B2, Lin_EOMs1, Lin_EOMs2] = Derive_4th_Ordr_Lin_Sys(obj)
            %Derive_4th_Ordr_Lin_Sys Separate the uncoupled linear equations of the 8th
            %order system into two entirely separate 4th order linear systems
            %   Detailed explanation goes here
            
            %Extract out the elements from A_uc and B_uc associated with the two decoupled
            %4th order systems
            obj.A1 = obj.A(1:4,1:4);
            obj.B1 = obj.B(1:4,1);
            obj.A2 = obj.A(5:8,5:8);
            obj.B2 = obj.B(5:8,2);

            %Create symbolic equation object for each of these two systems (used later in
            %control system design)
            obj.Lin_EOMs1 = obj.VDefs.stateVec1_dot == obj.A1*obj.VDefs.stateVec1 + obj.B1*obj.VDefs.T_beta;
            obj.Lin_EOMs2 = obj.VDefs.stateVec2_dot == obj.A2*obj.VDefs.stateVec2 + obj.B2*obj.VDefs.T_gamma;
            
            %Return Outputs
            A1 = obj.A1;
            B1 = obj.B1;
            A2 = obj.A2;
            B2 = obj.B2;
            Lin_EOMs1 = obj.Lin_EOMs1;
            Lin_EOMs2 = obj.Lin_EOMs2;
            
        end

        function [] = Derive_TF_y1(obj)

            %Get system transfer function
            syms epsilon
            obj.C_y1 = sym(zeros(1,8));
            obj.C_y1(1) = 1;
            obj.C_y1(3) = epsilon;
            obj.D_y1 = sym(zeros(1,2));
            obj.TF_y1 = simplify(obj.C_y1*inv(obj.VDefs.s*eye(8) - obj.A)*obj.B + obj.D_y1); 

        end

        
    end
end

