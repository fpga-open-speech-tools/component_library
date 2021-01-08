pipeline
{
    agent none
    stages
    {
        stage ('Ubuntu 20.04')
        {
            agent {label 'Ubuntu_20.04.1'}
            stages 
            {
                stage('Check the AD1939 Driver') 
                {
                    when { 
                        anyOf {
                            changeset "ad1939/*.vhd"
                            changeset "ad1939/*.tcl"
                        }
                    steps 
                    {
                        build job: 'DE10_AudioMini_Passthrough'
                    }

                }

                stage('Check the LKMs')
                {
                    when 
                    {
                        anyOf 
                        {
                            changeset "ad1939/FE_AD1939.c"
                            changeset "ad7768/FE_AD7768_4.c"
                            changeset "pga2505/FE_PGA2505.c"
                            changeset "tpa613a2/FE_TPA613A2.c"
                        }
                    }
                    stages
                    {
                        build job: 'Linux_LKMs'
                    }
                }
                    
                stage('Cleanup')
                {
                    steps
                    {
                        deleteDir()
                    }
                } 
            }
        }
    }
}