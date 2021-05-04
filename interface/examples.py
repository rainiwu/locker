import lockpy

def loopback(outname="pyloopback.iq", freq=2.4e9, rxgain=20.0, txgain=10.0):
    '''
    Simultaneous TX/RX. Should output a continuous complex sinusoid.
    :param outname: name of output file
    :type outname: str
    :param freq: TX/RX frequency in hertz
    :type freq: float
    :param rxgain: rxgain in dB
    :type rxgain: float
    :param txgain: txgain in dB
    :type txgain: float
    '''
    assert isinstance(outname, str)
    assert isinstance(freq, float)
    assert isinstance(rxgain, float)
    assert isinstance(txgain, float)

    assert len(outname) > 0

    # initialize wrapper
    anInstance = lockpy.PyLock()

    # set arguments
    anInstance.rxfile = outname
    anInstance.freq = freq
    anInstance.rxgain = rxgain
    anInstance.txgain = txgain

    anInstance.rxrate = 2e6
    anInstance.txrate = 2e6

    # initialize instance
    anInstance.make_instance()

    # queue commands
    anInstance.queue_rx(2e6)
    anInstance.queue_tx(2e6)

    # execute commands
    anInstance.execute(0.1, 0.0)
