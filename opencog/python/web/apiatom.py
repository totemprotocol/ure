__author__ = 'Cosmo Harrigan'

from flask import abort
from flask.ext.restful import Resource, reqparse, marshal
from opencog.atomspace import *
from mappers import *


class AtomAPI(Resource):
    @classmethod
    def new(cls, atomspace):
        cls.atomspace = atomspace
        return cls

    def __init__(self):
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('type', type=str, choices=types.__dict__.keys())
        self.reqparse.add_argument('name', type=str)
        super(AtomAPI, self).__init__()

    def get(self, id):
        """
        Returns the atom for the given handle
        Uri: atoms/[id]
        POST a JSON object as data with the following format:

        :param id: Atom handle
        :return atoms: Returns a JSON representation of an atom list containing the atom. Example:
        { 'atoms':
        { 'handle': 6, 'name': '', 'type': 'InheritanceLink', 'outgoing': [2, 1], 'incoming': [],
          'truthvalue': { 'type': 'simple',
          'details': { 'count': '0.4000000059604645', 'confidence': '0.0004997501382604241', 'strength': '0.5' }},
          'attentionvalue': { 'lti': 0, 'sti': 0, 'vlti': false }}}
        """
        try:
            atom = self.atomspace[Handle(id)]
        except IndexError:
            abort(404, 'Handle not found')

        return {'atoms': marshal(atom, atom_fields)}

    # @todo: add example & returns
    def put(self, id):
        """
        Updates the AttentionValue (STI, LTI, VLTI) or TruthValue of an atom
        Uri: atoms/[id]
        PUT a JSON object as data with the following format:

        :param truthvalue: (optional) TruthValue, formatted as follows:
            type TruthValue type (only 'simple' is currently available), see http://wiki.opencog.org/w/TruthValue
            details TruthValue parameters, formatted as follows:
                strength
                count
        :param attentionvalue: (optional) AttentionValue, formatted as follows:
            sti (optional) Short-Term Importance
            lti (optional) Long-Term Importance
            vlti (optional) Very-Long Term Importance

        :return atoms: Returns a JSON representation of an atom list containing the atom. Example:
        { 'atoms':
        { 'handle': 6, 'name': '', 'type': 'InheritanceLink', 'outgoing': [2, 1], 'incoming': [],
          'truthvalue': { 'type': 'simple',
          'details': { 'count': '0.4000000059604645', 'confidence': '0.0004997501382604241', 'strength': '0.5' }},
          'attentionvalue': { 'lti': 0, 'sti': 0, 'vlti': false }}}

        """

        # @todo: check if handle exists

        # Prepare the atom data
        data = reqparse.request.get_json()

        if 'truthvalue' not in data and 'attentionvalue' not in data:
            abort(400, 'Invalid request: you must include a truthvalue or attentionvalue parameter')

        if 'truthvalue' in data:
            tv = ParseTruthValue.parse(data)
            self.atomspace.set_tv(h=Handle(id), val=tv)

        if 'attentionvalue' in data:
            (sti, lti, vlti) = ParseAttentionValue.parse(data)
            self.atomspace.set_av(h=Handle(id), sti=sti, lti=lti, vlti=vlti)

        atom = self.atomspace[Handle(id)]
        return {'atoms': marshal(atom, atom_fields)}

    def delete(self, id):
        """
        Removes an atom from the AtomSpace
        Uri: atoms/[id]

        :param id: Atom handle
        :return result:  Returns a JSON representation of the result, indicating success or failure. Example:
        { 'result': { 'handle': 2, success: 'true' }}
        """

        try:
            atom = self.atomspace[Handle(id)]
        except IndexError:
            abort(404, 'Handle not found')
        status = self.atomspace.remove(atom)
        response = DeleteAtomResponse(id, status)
        return {'result': response.format()}
