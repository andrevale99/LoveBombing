idf_component_register(SRCS "LabMecFluMAIN.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver ads111x LabMecFluUART
                    LabMecFluPWM)
