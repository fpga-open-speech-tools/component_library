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
                        build job: 'Q18P0_DE10_AudioMini_Passthrough'
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
                        stage('Clone Frost Linux-SoCFPGA Repo')
                        {
                            steps
                            {
                                sh 'git clone https://github.com/fpga-open-speech-tools/linux-socfpga.git;'
                            }
                        }
                        stage('Configure Linux Kernel Build Env.')
                        {
                            steps
                            {   dir("linux-socfpga")
                                {
                                    sh 'make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- openspeech_defconfig'
                                    sh 'make prepare ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-'
                                }
                            }
                        }
                        stage('Build LKMs')
                        {
                            parallel
                            {
                                stage('AD1939 LKM')
                                {
                                    when { changeset "ad1939/FE_AD1939.c" }
                                    steps
                                    {   dir("ad1939")
                                        {
                                            sh 'make;'
                                            archiveArtifacts artifacts: '*.ko', fingerprint: true 
                                        }
                                    }
                                }
                                stage('AD7768 LKM')
                                {
                                    when { changeset "ad7768/FE_AD7768_4.c" }
                                    steps
                                    {   dir("ad7768")
                                        {
                                            sh 'make;'
                                            archiveArtifacts artifacts: '*.ko', fingerprint: true 
                                        }
                                    }
                                }
                                stage('PGA2505 LKM')
                                {
                                    when { changeset "pga2505/FE_PGA2505.c" }
                                    steps
                                    {   dir("pga2505")
                                        {
                                            sh 'make;'
                                            archiveArtifacts artifacts: '*.ko', fingerprint: true 
                                        }
                                    }
                                }
                                stage('TPA6160A2 LKM')
                                {
                                    when { changeset "tpa613a2/FE_TPA613A2.c" }
                                    steps
                                    {   dir("tpa613a2")
                                        {
                                            sh 'make;'
                                            archiveArtifacts artifacts: '*.ko', fingerprint: true 
                                        }
                                    }
                                }
                            }
                        }
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